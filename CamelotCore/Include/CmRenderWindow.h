/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE
-------------------------------------------------------------------------*/
#pragma once

#include "CmPrerequisites.h"

#include "CmRenderTarget.h"
#include "CmInt2.h"

namespace CamelotFramework
{
	enum class WindowBorder
	{
		Normal,
		None,
		Fixed
	};

	struct CM_EXPORT RENDER_WINDOW_DESC
	{
		RENDER_WINDOW_DESC()
			:width(0), height(0), fullscreen(false)
			, vsync(false), vsyncInterval(1), hidden(false)
			, displayFrequency(60), colorDepth(32), depthBuffer(true)
			, FSAA(0), FSAAHint(""), gamma(false), left(-1), top(-1)
			, title(""), border(WindowBorder::Normal), outerDimensions(false), enableDoubleClick(false)
			, monitorIndex(-1), toolWindow(false)
		{ }

		UINT32 width;
		UINT32 height;
		bool fullscreen;
		bool vsync;
		UINT32 vsyncInterval;
		bool hidden;
		UINT32 displayFrequency;
		UINT32 colorDepth;
		bool depthBuffer;
		UINT32 FSAA;
		String FSAAHint;
		bool gamma;
		INT32 left; // -1 == screen center
		INT32 top; // -1 == screen center
		String title;
		WindowBorder border;
		bool outerDimensions;
		bool enableDoubleClick;
		bool toolWindow;
		UINT32 monitorIndex; // -1 == select based on coordinates

		NameValuePairList platformSpecific;
	};

    class CM_EXPORT RenderWindow : public RenderTarget
    {
    public:
		virtual ~RenderWindow();

		/** 
		 *	@brief Core method. Alter fullscreen mode options.
		 */
		virtual void setFullscreen(bool fullScreen, UINT32 width, UINT32 height)
                { (void)fullScreen; (void)width; (void)height; }

        /**
         * @brief	Core method. Set the visibility state.
         */
        virtual void setHidden(bool hidden);

        /**
         * @brief	Core method. Alter the size of the window.
         */
        virtual void resize(UINT32 width, UINT32 height) = 0;

        /**
         * @brief	Core method. Reposition the window.
         */
        virtual void move(INT32 left, INT32 top) = 0;

		/**
		 * @copydoc RenderTarget::isWindow.
		 */
		bool isWindow() const { return true; }

        /**
         * @brief	Indicates whether the window is visible (not minimized or obscured).
         */
        virtual bool isVisible(void) const { return true; }

        /** 
        * @copydoc RenderTarget::isActive
		*/
        virtual bool isActive() const { return mActive && isVisible(); }

        /** 
        * @brief Indicates whether the window has been closed by the user.
		*/
        virtual bool isClosed() const = 0;
        
        /** 
        * @brief Returns true if window is running in fullscreen mode.
		*/
        virtual bool isFullScreen() const;

		INT32 getLeft() const { return mLeft; }
		INT32 getTop() const { return mTop; }

		/**
		 * @brief	Indicates whether the window currently has keyboard focus.
		 */
		bool hasFocus() const { return mHasFocus; }

		virtual Int2 screenToWindowPos(const Int2& screenPos) const = 0;
		virtual Int2 windowToScreenPos(const Int2& windowPos) const = 0;

		virtual void destroy();

		static RenderWindowPtr create(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow = nullptr);

    protected:
		friend class RenderWindowManager;

        RenderWindow(const RENDER_WINDOW_DESC& desc);

		/**
         * @brief	Internal method. Core method. Called when window is moved or resized.
         */
        virtual void _windowMovedOrResized();

		/**
         * @brief	Internal method. Core method. Called when window has received focus.
         */
		virtual void _windowFocusReceived();

		/**
         * @brief	Internal method. Core method. Called when window has lost focus.
         */
		virtual void _windowFocusLost();
        
	protected:
		bool mIsFullScreen;
		INT32 mLeft;
		INT32 mTop;
		bool mHasFocus;
		bool mHidden;

		RENDER_WINDOW_DESC mDesc;
    };
}
