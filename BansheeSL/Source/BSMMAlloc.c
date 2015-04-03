#include "BsMMAlloc.h"
#include <stdlib.h>
#include <string.h>

typedef struct tagMMAllocHeader MMAllocHeader;

struct tagMMAllocHeader
{
	MMAllocHeader* next;
	MMAllocHeader* prev;
};

void* mmalloc_new_context()
{
	MMAllocHeader* header = (MMAllocHeader*)malloc(sizeof(MMAllocHeader));
	header->next = 0;
	header->prev = 0;

	return header;
}

void mmalloc_free_context(void* context)
{
	MMAllocHeader* header = (MMAllocHeader*)context;
	while (header->next != 0)
		mmfree(header->next);

	free(header);
}

void* mmalloc(void* context, int size)
{
	void* buffer = malloc(size + sizeof(MMAllocHeader));

	MMAllocHeader* header = (MMAllocHeader*)buffer;
	MMAllocHeader* parent = (MMAllocHeader*)context;

	header->next = parent->next;
	header->prev = parent;
	parent->next = header;

	return (char*)buffer + sizeof(MMAllocHeader);
}

void mmfree(void* ptr)
{
	void* buffer = (char*)ptr - sizeof(MMAllocHeader);
	MMAllocHeader* header = (MMAllocHeader*)buffer;

	if (header->prev)
		header->prev->next = header->next;

	if (header->next)
		header->next->prev = header->prev;

	free(buffer);
}

char* mmalloc_strdup(void* context, const char* input)
{
	size_t length = strlen(input);
	char* output = mmalloc(context, sizeof(char) * (length + 1));

	memcpy(output, input, length);
	output[length] = '\0';

	return output;
}