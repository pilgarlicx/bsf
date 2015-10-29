#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsPrefabDiff.h"
#include "BsSerializedObject.h"
#include "BsGameObjectManager.h"
#include "BsBinarySerializer.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT PrefabComponentDiffRTTI : public RTTIType < PrefabComponentDiff, IReflectable, PrefabComponentDiffRTTI >
	{
	private:
		BS_PLAIN_MEMBER(id)
		BS_REFLPTR_MEMBER(data);
	public:
		PrefabComponentDiffRTTI()
		{
			BS_ADD_PLAIN_FIELD(id, 0);
			BS_ADD_REFLPTR_FIELD(data, 1);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "PrefabComponentDiff";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_PrefabComponentDiff;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<PrefabComponentDiff>();
		}
	};

	class BS_CORE_EXPORT PrefabObjectDiffRTTI : public RTTIType < PrefabObjectDiff, IReflectable, PrefabObjectDiffRTTI >
	{
	private:
		BS_PLAIN_MEMBER(id)
		BS_PLAIN_MEMBER(name)

		BS_REFLPTR_MEMBER_VEC(componentDiffs)
		BS_PLAIN_MEMBER_VEC(removedComponents)
		BS_REFLPTR_MEMBER_VEC(addedComponents)

		BS_REFLPTR_MEMBER_VEC(childDiffs)
		BS_PLAIN_MEMBER_VEC(removedChildren)
		BS_REFLPTR_MEMBER_VEC(addedChildren)
	public:
		PrefabObjectDiffRTTI()
		{
			BS_ADD_PLAIN_FIELD(id, 0);
			BS_ADD_PLAIN_FIELD(name, 1);

			BS_ADD_REFLPTR_FIELD_ARR(componentDiffs, 2);
			BS_ADD_PLAIN_FIELD_ARR(removedComponents, 3);
			BS_ADD_REFLPTR_FIELD_ARR(addedComponents, 4);

			BS_ADD_REFLPTR_FIELD_ARR(childDiffs, 5);
			BS_ADD_PLAIN_FIELD_ARR(removedChildren, 6);
			BS_ADD_REFLPTR_FIELD_ARR(addedChildren, 7);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "PrefabObjectDiff";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_PrefabObjectDiff;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<PrefabObjectDiff>();
		}
	};

	class BS_CORE_EXPORT PrefabDiffRTTI : public RTTIType < PrefabDiff, IReflectable, PrefabDiffRTTI >
	{
		/**
		 * @brief	Contains data about a game object handle serialized in a prefab diff. 
		 */
		struct SerializedHandle
		{
			SPtr<SerializedObject> object;
			SPtr<GameObjectHandleBase> handle;
		};

	private:
		BS_REFLPTR_MEMBER(mRoot);
	public:
		PrefabDiffRTTI()
		{
			BS_ADD_REFLPTR_FIELD(mRoot, 0);
		}

		virtual void onDeserializationStarted(IReflectable* obj) override
		{
			PrefabDiff* prefabDiff = static_cast<PrefabDiff*>(obj);

			if (GameObjectManager::instance().isGameObjectDeserializationActive())
				GameObjectManager::instance().registerOnDeserializationEndCallback(std::bind(&PrefabDiffRTTI::delayedOnDeserializationEnded, prefabDiff));
		}

		virtual void onDeserializationEnded(IReflectable* obj) override
		{
			assert(GameObjectManager::instance().isGameObjectDeserializationActive());

			// Make sure to deserialize all game object handles since their IDs need to be updated. Normally they are
			// updated automatically upon deserialization but since we store them in intermediate form we need to manually
			// deserialize and reserialize them in order to update their IDs.
			PrefabDiff* prefabDiff = static_cast<PrefabDiff*>(obj);

			Stack<SPtr<PrefabObjectDiff>> todo;

			if (prefabDiff->mRoot != nullptr)
				todo.push(prefabDiff->mRoot);

			UnorderedSet<SPtr<SerializedObject>> handleObjects;

			while (!todo.empty())
			{
				SPtr<PrefabObjectDiff> current = todo.top();
				todo.pop();

				for (auto& component : current->addedComponents)
					findGameObjectHandles(component, handleObjects);

				for (auto& child : current->addedChildren)
					findGameObjectHandles(child, handleObjects);

				for (auto& component : current->componentDiffs)
					findGameObjectHandles(component->data, handleObjects);

				for (auto& child : current->childDiffs)
					todo.push(child);
			}

			Vector<SerializedHandle> handleData(handleObjects.size());

			UINT32 idx = 0;
			BinarySerializer bs;
			for (auto& handleObject : handleObjects)
			{
				SerializedHandle& handle = handleData[idx];

				handle.object = handleObject;
				handle.handle = std::static_pointer_cast<GameObjectHandleBase>(bs._decodeIntermediate(handleObject));

				idx++;
			}

			prefabDiff->mRTTIData = handleData;
		}

		/**
		 * @brief	Decodes GameObjectHandles from their binary format, because during deserialization GameObjectManager
		 *			will update all object IDs and we want to keep the handles up to date.So we deserialize them
		 *			and allow them to be updated before storing them back into binary format.
		 */
		static void delayedOnDeserializationEnded(PrefabDiff* prefabDiff)
		{
			Vector<SerializedHandle>& handleData = any_cast_ref<Vector<SerializedHandle>>(prefabDiff->mRTTIData);

			BinarySerializer bs;
			for (auto& serializedHandle : handleData)
			{
				if (serializedHandle.handle != nullptr)
					*serializedHandle.object = *bs._encodeIntermediate(serializedHandle.handle.get());
			}

			prefabDiff->mRTTIData = nullptr;
		}

		/**
		 * @brief	Scans the entire hierarchy and find all serialized GameObjectHandle objects.
		 */
		static void findGameObjectHandles(const SPtr<SerializedObject>& serializedObject, UnorderedSet<SPtr<SerializedObject>>& handleObjects)
		{
			for (auto& subObject : serializedObject->subObjects)
			{
				RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
				if (rtti == nullptr)
					continue;

				if (rtti->getRTTIId() == TID_GameObjectHandleBase)
				{
					handleObjects.insert(serializedObject);
					return;
				}

				for (auto& child : subObject.entries)
				{
					RTTIField* curGenericField = rtti->getField(child.second.fieldId);
					if (curGenericField == nullptr)
						continue;

					SPtr<SerializedInstance> entryData = child.second.serialized;
					if (curGenericField->isArray())
					{
						SPtr<SerializedArray> arrayData = std::static_pointer_cast<SerializedArray>(entryData);

						switch (curGenericField->mType)
						{
						case SerializableFT_ReflectablePtr:
						case SerializableFT_Reflectable:
						{
							for (auto& arrayElem : arrayData->entries)
							{
								SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.serialized);

								if (arrayElemData != nullptr)
									findGameObjectHandles(arrayElemData, handleObjects);
							}
						}
							break;
						}
					}
					else
					{
						switch (curGenericField->mType)
						{
						case SerializableFT_ReflectablePtr:
						case SerializableFT_Reflectable:
						{
							SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(entryData);
							if (fieldObjectData != nullptr)
								findGameObjectHandles(fieldObjectData, handleObjects);
						}
							break;
						}
					}
				}
			}
		}

		virtual const String& getRTTIName() override
		{
			static String name = "PrefabDiff";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_PrefabDiff;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<PrefabDiff>();
		}
	};
}