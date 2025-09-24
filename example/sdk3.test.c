
// Everything IPC test
// this tests the lib and the dll.

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define EVERYTHING3_USERAPI

// includes
#include <windows.h>
#include <shlobj.h> // Propvariant

#include "Everything3.h"

static EVERYTHING3_CLIENT *test_Connect(void);

void hexdump(const void *data,uintptr_t size)
{
	const BYTE *p;
	uintptr_t run;
	
	p = data;
	run = size;
	
	while(run)
	{
		printf("%02X",*p);
		
		p++;
		run--;
	}
}

void simple_example(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		EVERYTHING3_SEARCH_STATE *search_state;

		// Connected..
		
		// Create a empty search state.
		search_state = Everything3_CreateSearchState();
		if (search_state)
		{
			EVERYTHING3_RESULT_LIST *result_list;

			// Set the search text.
			Everything3_SetSearchTextW(search_state,L"ABC 123");
			
			// Execute the search.
			result_list = Everything3_Search(client,search_state);
			if (result_list)
			{
				SIZE_T viewport_count;
				SIZE_T result_index;
				
				// Search successful.

				// Get the number of results.
				viewport_count = Everything3_GetResultListViewportCount(result_list);
				
				// loop through the results.
				for(result_index=0;result_index<viewport_count;result_index++)
				{
					wchar_t filename[MAX_PATH];

					// Get the filename
					Everything3_GetResultFullPathNameW(result_list,result_index,filename,MAX_PATH);
					
					// Display the filename.
					printf("%S\n",filename);
				}
				
				// Destroy the result list.
				Everything3_DestroyResultList(result_list);
			}

			// Destroy the search state.
			Everything3_DestroySearchState(search_state);
		}
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
}

void test_viewport(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		EVERYTHING3_SEARCH_STATE *search_state;

		// Connected..
		
		// Create a empty search state.
		search_state = Everything3_CreateSearchState();
		if (search_state)
		{
			EVERYTHING3_RESULT_LIST *result_list;

			// Set the search text.
			Everything3_SetSearchTextW(search_state,L"sonic tails");

			Everything3_SetSearchViewportOffset(search_state,10);
			Everything3_SetSearchViewportCount(search_state,3);
			
			// Execute the search.
			result_list = Everything3_Search(client,search_state);
			if (result_list)
			{
				SIZE_T viewport_count;
				SIZE_T result_index;
				
				// Search successful.

				// Get the number of results.
				viewport_count = Everything3_GetResultListViewportCount(result_list);
				
				// loop through the results.
				for(result_index=0;result_index<viewport_count;result_index++)
				{
					wchar_t filename[MAX_PATH];

					// Get the filename
					Everything3_GetResultFullPathNameW(result_list,result_index,filename,MAX_PATH);
					
					// Display the filename.
					printf("%S\n",filename);
				}
				
				// Destroy the result list.
				Everything3_DestroyResultList(result_list);
			}

			// Destroy the search state.
			Everything3_DestroySearchState(search_state);
		}
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
}

void test_GetFileAttributesEx(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
//		WIN32_FIND_DATAW fd;
		WIN32_FIND_DATAA ansi_fd;
/*		
//		if (Everything3_GetFileAttributesExW(client,L"C:\\Windows\\Notepad.exe",&fd))
		if (Everything3_GetFileAttributesExW(client,L"C:\\Windows\\system32",&fd))
//		if (Everything3_GetFileAttributesExW(client,L"C:\\Windows",&fd))
		{
			printf("Everything3_GetFileAttributesExW dwFileAttributes %08x\n",fd.dwFileAttributes);
			printf("Everything3_GetFileAttributesExW ftCreationTime.dwHighDateTime %08x\n",fd.ftCreationTime.dwHighDateTime);
			printf("Everything3_GetFileAttributesExW ftCreationTime.dwLowDateTime %08x\n",fd.ftCreationTime.dwLowDateTime);
			printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwHighDateTime %08x\n",fd.ftLastAccessTime.dwHighDateTime);
			printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwLowDateTime %08x\n",fd.ftLastAccessTime.dwLowDateTime);
			printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwHighDateTime %08x\n",fd.ftLastWriteTime.dwHighDateTime);
			printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwLowDateTime %08x\n",fd.ftLastWriteTime.dwLowDateTime);
			printf("Everything3_GetFileAttributesExW nFileSizeHigh %08x\n",fd.nFileSizeHigh);
			printf("Everything3_GetFileAttributesExW nFileSizeLow %08x\n",fd.nFileSizeLow);
			printf("Everything3_GetFileAttributesExW dwReserved0 %08x\n",fd.dwReserved0);
			printf("Everything3_GetFileAttributesExW dwReserved1 %08x\n",fd.dwReserved1);
			printf("Everything3_GetFileAttributesExW cFileName %S\n",fd.cFileName);
			printf("Everything3_GetFileAttributesExW cAlternateFileName %S\n",fd.cAlternateFileName);
		}
		else
		{
			printf("Everything3_GetFileAttributesExW failed %08x\n",Everything3_GetLastError());
		}
*/			
		
//		if (Everything3_GetFileAttributesExW(client,L"C:\\Windows\\Notepad.exe",&fd))
		if (Everything3_GetFileAttributesExA(client,"C:\\Windows\\system32",&ansi_fd))
//		if (Everything3_GetFileAttributesExW(client,L"C:\\Windows",&fd))
		{
			printf("Everything3_GetFileAttributesExW dwFileAttributes %08x\n",ansi_fd.dwFileAttributes);
			printf("Everything3_GetFileAttributesExW ftCreationTime.dwHighDateTime %08x\n",ansi_fd.ftCreationTime.dwHighDateTime);
			printf("Everything3_GetFileAttributesExW ftCreationTime.dwLowDateTime %08x\n",ansi_fd.ftCreationTime.dwLowDateTime);
			printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwHighDateTime %08x\n",ansi_fd.ftLastAccessTime.dwHighDateTime);
			printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwLowDateTime %08x\n",ansi_fd.ftLastAccessTime.dwLowDateTime);
			printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwHighDateTime %08x\n",ansi_fd.ftLastWriteTime.dwHighDateTime);
			printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwLowDateTime %08x\n",ansi_fd.ftLastWriteTime.dwLowDateTime);
			printf("Everything3_GetFileAttributesExW nFileSizeHigh %08x\n",ansi_fd.nFileSizeHigh);
			printf("Everything3_GetFileAttributesExW nFileSizeLow %08x\n",ansi_fd.nFileSizeLow);
			printf("Everything3_GetFileAttributesExW dwReserved0 %08x\n",ansi_fd.dwReserved0);
			printf("Everything3_GetFileAttributesExW dwReserved1 %08x\n",ansi_fd.dwReserved1);
			printf("Everything3_GetFileAttributesExW cFileName %s\n",ansi_fd.cFileName);
			printf("Everything3_GetFileAttributesExW cAlternateFileName %s\n",ansi_fd.cAlternateFileName);
		}
		else
		{
			printf("Everything3_GetFileAttributesExW failed %08x\n",Everything3_GetLastError());
		}
		
		printf("C:\\Windows\\system32 attr %08x\n",Everything3_GetFileAttributesW(client,L"C:\\Windows\\system32"));
		printf("C:\\Windows\\system32x attr %08x\n",Everything3_GetFileAttributesW(client,L"C:\\Windows\\system32x"));
		printf("C:\\Windows\\notepad.exe attr %08x\n",Everything3_GetFileAttributesW(client,L"C:\\Windows\\notepad.exe"));
		
		printf("C:\\Windows\\system32 attr %08x\n",Everything3_GetFileAttributesA(client,"C:\\Windows\\system32"));
		printf("C:\\Windows\\system32x attr %08x\n",Everything3_GetFileAttributesA(client,"C:\\Windows\\system32x"));
		printf("C:\\Windows\\notepad.exe attr %08x\n",Everything3_GetFileAttributesA(client,"C:\\Windows\\notepad.exe"));
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
}

void test_FindFirstFile(void)
{
	EVERYTHING3_CLIENT *client;
/*
	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		EVERYTHING3_FIND_HANDLE *find_handle;
		WIN32_FIND_DATA fd;
		
		find_handle = Everything3_FindFirstFileW(client,L"C:\\meta\\*",&fd);
		if (find_handle)
		{
			for(;;)
			{
				printf("Everything3_GetFileAttributesExW dwFileAttributes %08x\n",fd.dwFileAttributes);
				printf("Everything3_GetFileAttributesExW ftCreationTime.dwHighDateTime %08x\n",fd.ftCreationTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftCreationTime.dwLowDateTime %08x\n",fd.ftCreationTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwHighDateTime %08x\n",fd.ftLastAccessTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwLowDateTime %08x\n",fd.ftLastAccessTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwHighDateTime %08x\n",fd.ftLastWriteTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwLowDateTime %08x\n",fd.ftLastWriteTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW nFileSizeHigh %08x\n",fd.nFileSizeHigh);
				printf("Everything3_GetFileAttributesExW nFileSizeLow %08x\n",fd.nFileSizeLow);
				printf("Everything3_GetFileAttributesExW dwReserved0 %08x\n",fd.dwReserved0);
				printf("Everything3_GetFileAttributesExW dwReserved1 %08x\n",fd.dwReserved1);
				printf("Everything3_GetFileAttributesExW cFileName %S\n",fd.cFileName);
				printf("Everything3_GetFileAttributesExW cAlternateFileName %S\n",fd.cAlternateFileName);
				
				if (!Everything3_FindNextFileW(find_handle,&fd))
				{
					break;
				}
			}
			
			Everything3_FindClose(find_handle);
		}
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
*/
/*	
	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		EVERYTHING3_FIND_HANDLE *find_handle;
		WIN32_FIND_DATAA fd;
		
		find_handle = Everything3_FindFirstFileA(client,"C:\\meta\\*",&fd);
		if (find_handle)
		{
			for(;;)
			{
				printf("Everything3_GetFileAttributesExW dwFileAttributes %08x\n",fd.dwFileAttributes);
				printf("Everything3_GetFileAttributesExW ftCreationTime.dwHighDateTime %08x\n",fd.ftCreationTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftCreationTime.dwLowDateTime %08x\n",fd.ftCreationTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwHighDateTime %08x\n",fd.ftLastAccessTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwLowDateTime %08x\n",fd.ftLastAccessTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwHighDateTime %08x\n",fd.ftLastWriteTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwLowDateTime %08x\n",fd.ftLastWriteTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW nFileSizeHigh %08x\n",fd.nFileSizeHigh);
				printf("Everything3_GetFileAttributesExW nFileSizeLow %08x\n",fd.nFileSizeLow);
				printf("Everything3_GetFileAttributesExW dwReserved0 %08x\n",fd.dwReserved0);
				printf("Everything3_GetFileAttributesExW dwReserved1 %08x\n",fd.dwReserved1);
				printf("Everything3_GetFileAttributesExW cFileName %s\n",fd.cFileName);
				printf("Everything3_GetFileAttributesExW cAlternateFileName %s\n",fd.cAlternateFileName);
				
				if (!Everything3_FindNextFileA(find_handle,&fd))
				{
					break;
				}
			}
			
			Everything3_FindClose(find_handle);
		}
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
	*/
	/*
	// connect to Everything..	
	client = Everything3_Connect(L"pipename%\\:check");
	if (client)
	{
		EVERYTHING3_FIND_HANDLE *find_handle;
		WIN32_FIND_DATAA fd;
		
		find_handle = Everything3_FindFirstFileA(client,"D:\\downloads\\*",&fd);
		if (find_handle)
		{
			for(;;)
			{
				printf("Everything3_GetFileAttributesExW dwFileAttributes %08x\n",fd.dwFileAttributes);
				printf("Everything3_GetFileAttributesExW ftCreationTime.dwHighDateTime %08x\n",fd.ftCreationTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftCreationTime.dwLowDateTime %08x\n",fd.ftCreationTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwHighDateTime %08x\n",fd.ftLastAccessTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwLowDateTime %08x\n",fd.ftLastAccessTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwHighDateTime %08x\n",fd.ftLastWriteTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwLowDateTime %08x\n",fd.ftLastWriteTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW nFileSizeHigh %08x\n",fd.nFileSizeHigh);
				printf("Everything3_GetFileAttributesExW nFileSizeLow %08x\n",fd.nFileSizeLow);
				printf("Everything3_GetFileAttributesExW dwReserved0 %08x\n",fd.dwReserved0);
				printf("Everything3_GetFileAttributesExW dwReserved1 %08x\n",fd.dwReserved1);
				printf("Everything3_GetFileAttributesExW cFileName %s\n",fd.cFileName);
				printf("Everything3_GetFileAttributesExW cAlternateFileName %s\n",fd.cAlternateFileName);
				
				if (!Everything3_FindNextFileA(find_handle,&fd))
				{
					break;
				}
			}
			
			Everything3_FindClose(find_handle);
		}
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
	*/
	
	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		EVERYTHING3_FIND_HANDLE *find_handle;
		WIN32_FIND_DATAA fd;
		
		find_handle = Everything3_FindFirstFileA(client,"C:\\META\\DEEPPATH\\00\\01\\02\\03\\04\\05\\06\\07\\08\\09\\0A\\0B\\0C\\0D\\0E\\0F\\10\\11\\12\\13\\14\\15\\16\\17\\18\\19\\1A\\1B\\1C\\1D\\1E\\1F\\20\\21\\22\\23\\24\\25\\26\\27\\28\\29\\2A\\2B\\2C\\2D\\2E\\2F\\30\\31\\32\\33\\34\\35\\36\\37\\38\\39\\3A\\3B\\3C\\3D\\3E\\3F\\40\\41\\42\\43\\44\\45\\46\\47\\48\\49\\4A\\4B\\4C\\4D\\4E\\4F\\50\\51\\52\\53\\54\\55\\56\\57\\58\\59\\5A\\5B\\5C\\5D\\5E\\5F\\60\\61\\62\\63\\64\\65\\66\\67\\68\\69\\6A\\6B\\6C\\6D\\6E\\6F\\70\\71\\72\\73\\74\\75\\76\\77\\78\\79\\7A\\7B\\7C\\7D\\7E\\7F\\80\\81\\82\\83\\84\\85\\86\\87\\88\\89\\8A\\8B\\8C\\8D\\8E\\8F\\90\\91\\92\\93\\94\\95\\96\\97\\98\\99\\9A\\9B\\9C\\9D\\9E\\9F\\A0\\A1\\A2\\A3\\A4\\A5\\A6\\A7\\A8\\A9\\AA\\AB\\AC\\AD\\AE\\AF\\B0\\B1\\B2\\B3\\B4\\B5\\B6\\B7\\B8\\B9\\BA\\BB\\BC\\BD\\BE\\BF\\C0\\C1\\C2\\C3\\C4\\C5\\C6\\C7\\C8\\C9\\CA\\CB\\CC\\CD\\CE\\CF\\D0\\D1\\D2\\D3\\D4\\D5\\D6\\D7\\D8\\D9\\DA\\DB\\DC\\DD\\DE\\DF\\E0\\E1\\E2\\E3\\E4\\E5\\E6\\E7\\E8\\E9\\EA\\EB\\EC\\ED\\EE\\EF\\F0\\F1\\F2\\F3\\F4\\F5\\F6\\F7\\F8\\F9\\FA\\FB\\FC\\FD\\FE\\*",&fd);
		if (find_handle)
		{
			for(;;)
			{
				printf("Everything3_GetFileAttributesExW dwFileAttributes %08x\n",fd.dwFileAttributes);
				printf("Everything3_GetFileAttributesExW ftCreationTime.dwHighDateTime %08x\n",fd.ftCreationTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftCreationTime.dwLowDateTime %08x\n",fd.ftCreationTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwHighDateTime %08x\n",fd.ftLastAccessTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftLastAccessTime.dwLowDateTime %08x\n",fd.ftLastAccessTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwHighDateTime %08x\n",fd.ftLastWriteTime.dwHighDateTime);
				printf("Everything3_GetFileAttributesExW ftLastWriteTime.dwLowDateTime %08x\n",fd.ftLastWriteTime.dwLowDateTime);
				printf("Everything3_GetFileAttributesExW nFileSizeHigh %08x\n",fd.nFileSizeHigh);
				printf("Everything3_GetFileAttributesExW nFileSizeLow %08x\n",fd.nFileSizeLow);
				printf("Everything3_GetFileAttributesExW dwReserved0 %08x\n",fd.dwReserved0);
				printf("Everything3_GetFileAttributesExW dwReserved1 %08x\n",fd.dwReserved1);
				printf("Everything3_GetFileAttributesExW cFileName %s\n",fd.cFileName);
				printf("Everything3_GetFileAttributesExW cAlternateFileName %s\n",fd.cAlternateFileName);
				
				if (!Everything3_FindNextFileA(find_handle,&fd))
				{
					break;
				}
			}
			
			Everything3_FindClose(find_handle);
		}
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
	
	
}

static EVERYTHING3_CLIENT *test_Connect(void)
{
	EVERYTHING3_CLIENT *client;
	
	client = Everything3_Connect(NULL);
	if (client)
	{
		return client;
	}
	
	client = Everything3_Connect(L"1.5a");
	if (client)
	{
		return client;
	}

	// VOIDs metadata test instance.
	client = Everything3_Connect(L"meta");
	if (client)
	{
		return client;
	}
	
	// VOIDs 1.5 release test instance.
	client = Everything3_Connect(L"1.5");
	if (client)
	{
		return client;
	}
	
	printf("Everything3_Connect failed %u\n",GetLastError());
	
	return NULL;
}

void test_GetJournalInfo(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		EVERYTHING3_JOURNAL_INFO jorunal_info;
		
		if (Everything3_GetJournalInfo(client,&jorunal_info))
		{
			printf("journal_id %016I64x\n",jorunal_info.journal_id);
			printf("first_item_index %016I64x\n",jorunal_info.first_item_index);
			printf("next_item_index %016I64x\n",jorunal_info.next_item_index);
			printf("size %016I64x\n",jorunal_info.size);
			printf("max_size %016I64x\n",jorunal_info.max_size);
		}
		else
		{
			printf("Everything3_GetJournalInfo failed %u\n",GetLastError());
		}

		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
}

BOOL WINAPI test_ReadJournalCallbackProc(void *user_data,const EVERYTHING3_JOURNAL_CHANGE *journal_change)
{
	char *type_name;

	printf("Journal Change\n");
	printf("Journal ID: %016I64x\n",journal_change->journal_id);
	printf("Change ID: %016I64x\n",journal_change->change_id);
	
	type_name = "Unknown Type";
	
	switch(journal_change->type)
	{
		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FOLDER_CREATE:
			type_name = "Folder Create";
			break;

		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FOLDER_DELETE:
			type_name = "Folder Delete";
			break;

		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FOLDER_RENAME:
			type_name = "Folder Rename";
			break;

		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FOLDER_MOVE:
			type_name = "Folder Move";
			break;

		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FOLDER_MODIFY:
			type_name = "Folder Modify";
			break;

		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FILE_CREATE:
			type_name = "File Create";
			break;

		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FILE_DELETE:
			type_name = "File Delete";
			break;

		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FILE_RENAME:
			type_name = "File Rename";
			break;

		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FILE_MOVE:
			type_name = "File Move";
			break;

		case EVERYTHING3_JOURNAL_CHANGE_TYPE_FILE_MODIFY:
			type_name = "File Modify";
			break;
			
	}

	printf("Type: %d: %s\n",journal_change->type,type_name);
	printf("Date: %016I64x\n",journal_change->timestamp);
	printf("Source Date: %016I64x\n",journal_change->source_timestamp);
	printf("Old Parent Date Modified: %016I64x\n",journal_change->old_parent_date_modified);
	printf("New Parent Date Modified: %016I64x\n",journal_change->new_parent_date_modified);
	printf("Size: %I64u\n",journal_change->size);
	printf("Date Created: %016I64x\n",journal_change->date_created);
	printf("Date Modified: %016I64x\n",journal_change->date_modified);
	printf("Date Accessed: %016I64x\n",journal_change->date_accessed);
	printf("Attributes: %08x\n",journal_change->attributes);
	printf("Old Path: %S\n",journal_change->old_path);
	printf("Old Name: %S\n",journal_change->old_name);
	printf("New Path: %S\n",journal_change->new_path);
	printf("New Name: %S\n",journal_change->new_name);
	
	// continue.
	return TRUE;
}

void test_ReadJournal(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		EVERYTHING3_JOURNAL_INFO jorunal_info;
		
		if (Everything3_GetJournalInfo(client,&jorunal_info))
		{
			if (Everything3_ReadJournal(client,
				jorunal_info.journal_id,
				EVERYTHING3_UINT64_MAX,
				EVERYTHING3_READ_JOURNAL_FLAG_CHANGE_ID|
				EVERYTHING3_READ_JOURNAL_FLAG_TIMESTAMP|
				EVERYTHING3_READ_JOURNAL_FLAG_SOURCE_TIMESTAMP|
				EVERYTHING3_READ_JOURNAL_FLAG_OLD_PARENT_DATE_MODIFIED|
				EVERYTHING3_READ_JOURNAL_FLAG_OLD_PATH|
				EVERYTHING3_READ_JOURNAL_FLAG_OLD_NAME|
				EVERYTHING3_READ_JOURNAL_FLAG_SIZE|
				EVERYTHING3_READ_JOURNAL_FLAG_DATE_CREATED|
				EVERYTHING3_READ_JOURNAL_FLAG_DATE_MODIFIED|
				EVERYTHING3_READ_JOURNAL_FLAG_DATE_ACCESSED|
				EVERYTHING3_READ_JOURNAL_FLAG_ATTRIBUTES|
				EVERYTHING3_READ_JOURNAL_FLAG_NEW_PARENT_DATE_MODIFIED|
				EVERYTHING3_READ_JOURNAL_FLAG_NEW_PATH|
				EVERYTHING3_READ_JOURNAL_FLAG_NEW_NAME,
				NULL,
				test_ReadJournalCallbackProc))
			{
				// never reached.
			}
			else
			{
				printf("Everything3_ReadJournal failed %u\n",GetLastError());
			}
		}
		else
		{
			printf("Everything3_GetJournalInfo failed %u\n",GetLastError());
		}
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
}

void test_IsPropertyRightAligned(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		DWORD system_size;
		DWORD system_kind;
		DWORD system_keywords;
		DWORD system_media_duration;
		DWORD bad_property;
		
		printf("EVERYTHING3_PROPERTY_ID_NAME right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_NAME));
		printf("EVERYTHING3_PROPERTY_ID_PATH right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_PATH));
		printf("EVERYTHING3_PROPERTY_ID_SIZE right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_SIZE));
		printf("EVERYTHING3_PROPERTY_ID_EXTENSION right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_EXTENSION));
		printf("EVERYTHING3_PROPERTY_ID_DATE_MODIFIED right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_DATE_MODIFIED));
		printf("EVERYTHING3_PROPERTY_ID_DATE_CREATED right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_DATE_CREATED));
		printf("EVERYTHING3_PROPERTY_ID_DATE_ACCESSED right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_DATE_ACCESSED));
		printf("EVERYTHING3_PROPERTY_ID_ATTRIBUTES right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_ATTRIBUTES));
		printf("EVERYTHING3_PROPERTY_ID_WIDTH right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_WIDTH));
		printf("EVERYTHING3_PROPERTY_ID_LENGTH right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_LENGTH));
		printf("EVERYTHING3_PROPERTY_ID_GENRE right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_GENRE));
		printf("EVERYTHING3_PROPERTY_ID_TAGS right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_TAGS));
		printf("EVERYTHING3_PROPERTY_ID_CRC32 right aligned: %d\n",Everything3_IsPropertyRightAligned(client,EVERYTHING3_PROPERTY_ID_CRC32));

		system_size = Everything3_FindProperty(client,L"System.size");
		system_kind = Everything3_FindProperty(client,L"System.kind");
		system_keywords = Everything3_FindProperty(client,L"System.keywords");
		system_media_duration = Everything3_FindProperty(client,L"System.Media.Duration");
		bad_property = Everything3_FindProperty(client,L"abc");
		
		printf("System.size %d right aligned: %d\n",system_size,Everything3_IsPropertyRightAligned(client,system_size));
		printf("System.kind %d right aligned: %d\n",system_kind,Everything3_IsPropertyRightAligned(client,system_kind));
		printf("System.keywords %d right aligned: %d\n",system_keywords,Everything3_IsPropertyRightAligned(client,system_keywords));
		printf("System.Media.Duration %d right aligned: %d\n",system_media_duration,Everything3_IsPropertyRightAligned(client,system_media_duration));
		printf("bad_property %d right aligned: %d\n",bad_property,Everything3_IsPropertyRightAligned(client,bad_property));
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
}

void test_IsPropertySortDescending(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		DWORD system_size;
		DWORD system_kind;
		DWORD system_keywords;
		DWORD system_media_duration;
		DWORD bad_property;
		
		printf("EVERYTHING3_PROPERTY_ID_NAME sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_NAME));
		printf("EVERYTHING3_PROPERTY_ID_PATH sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_PATH));
		printf("EVERYTHING3_PROPERTY_ID_SIZE sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_SIZE));
		printf("EVERYTHING3_PROPERTY_ID_EXTENSION sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_EXTENSION));
		printf("EVERYTHING3_PROPERTY_ID_DATE_MODIFIED sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_DATE_MODIFIED));
		printf("EVERYTHING3_PROPERTY_ID_DATE_CREATED sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_DATE_CREATED));
		printf("EVERYTHING3_PROPERTY_ID_DATE_ACCESSED sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_DATE_ACCESSED));
		printf("EVERYTHING3_PROPERTY_ID_ATTRIBUTES sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_ATTRIBUTES));
		printf("EVERYTHING3_PROPERTY_ID_WIDTH sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_WIDTH));
		printf("EVERYTHING3_PROPERTY_ID_LENGTH sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_LENGTH));
		printf("EVERYTHING3_PROPERTY_ID_GENRE sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_GENRE));
		printf("EVERYTHING3_PROPERTY_ID_TAGS sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_TAGS));
		printf("EVERYTHING3_PROPERTY_ID_CRC32 sort descending: %d\n",Everything3_IsPropertySortDescending(client,EVERYTHING3_PROPERTY_ID_CRC32));

		system_size = Everything3_FindProperty(client,L"System.size");
		system_kind = Everything3_FindProperty(client,L"System.kind");
		system_keywords = Everything3_FindProperty(client,L"System.keywords");
		system_media_duration = Everything3_FindProperty(client,L"System.Media.Duration");
		bad_property = Everything3_FindProperty(client,L"abc");
		
		printf("System.size %d sort descending: %d\n",system_size,Everything3_IsPropertySortDescending(client,system_size));
		printf("System.kind %d sort descending: %d\n",system_kind,Everything3_IsPropertySortDescending(client,system_kind));
		printf("System.keywords %d sort descending: %d\n",system_keywords,Everything3_IsPropertySortDescending(client,system_keywords));
		printf("System.Media.Duration %d sort descending: %d\n",system_media_duration,Everything3_IsPropertySortDescending(client,system_media_duration));
		printf("bad_property %d sort descending: %d\n",bad_property,Everything3_IsPropertySortDescending(client,bad_property));
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
}

void test_GetPropertyDefaultWidth(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		DWORD system_size;
		DWORD system_kind;
		DWORD system_keywords;
		DWORD system_media_duration;
		DWORD bad_property;
		
		printf("EVERYTHING3_PROPERTY_ID_NAME default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_NAME));
		printf("EVERYTHING3_PROPERTY_ID_PATH default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_PATH));
		printf("EVERYTHING3_PROPERTY_ID_SIZE default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_SIZE));
		printf("EVERYTHING3_PROPERTY_ID_EXTENSION default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_EXTENSION));
		printf("EVERYTHING3_PROPERTY_ID_DATE_MODIFIED default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_DATE_MODIFIED));
		printf("EVERYTHING3_PROPERTY_ID_DATE_CREATED default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_DATE_CREATED));
		printf("EVERYTHING3_PROPERTY_ID_DATE_ACCESSED default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_DATE_ACCESSED));
		printf("EVERYTHING3_PROPERTY_ID_ATTRIBUTES default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_ATTRIBUTES));
		printf("EVERYTHING3_PROPERTY_ID_WIDTH default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_WIDTH));
		printf("EVERYTHING3_PROPERTY_ID_LENGTH default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_LENGTH));
		printf("EVERYTHING3_PROPERTY_ID_GENRE default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_GENRE));
		printf("EVERYTHING3_PROPERTY_ID_TAGS default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_TAGS));
		printf("EVERYTHING3_PROPERTY_ID_CRC32 default width: %d\n",Everything3_GetPropertyDefaultWidth(client,EVERYTHING3_PROPERTY_ID_CRC32));

		system_size = Everything3_FindProperty(client,L"System.size");
		system_kind = Everything3_FindProperty(client,L"System.kind");
		system_keywords = Everything3_FindProperty(client,L"System.keywords");
		system_media_duration = Everything3_FindProperty(client,L"System.Media.Duration");
		bad_property = Everything3_FindProperty(client,L"abc");
		
		printf("System.size %d default width: %d\n",system_size,Everything3_GetPropertyDefaultWidth(client,system_size));
		printf("System.kind %d default width: %d\n",system_kind,Everything3_GetPropertyDefaultWidth(client,system_kind));
		printf("System.keywords %d default width: %d\n",system_keywords,Everything3_GetPropertyDefaultWidth(client,system_keywords));
		printf("System.Media.Duration %d default width: %d\n",system_media_duration,Everything3_GetPropertyDefaultWidth(client,system_media_duration));
		printf("bad_property %d default width: %d\n",bad_property,Everything3_GetPropertyDefaultWidth(client,bad_property));
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
}

void test_PropertySystem(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything..	
	client = test_Connect();
	if (client)
	{
		DWORD system_size;
		DWORD system_kind;
		DWORD system_keywords;
		DWORD system_media_duration;
		DWORD bad_property;
		DWORD System_Image_HorizontalSize;
		EVERYTHING3_RESULT_LIST *result_list;
		EVERYTHING3_SEARCH_STATE *search_state;
		DWORD System_FileExtension;
		DWORD System_Null;
		DWORD System_ParsingBindContext;
		DWORD System_Image_ResolutionUnit;
		DWORD System_Document_LineCount;
		DWORD System_GPS_Altitude;
		DWORD System_Title;
		DWORD System_Audio_IsVariableBitRate;
		DWORD System_GPS_AltitudeRef;
		DWORD System_Photo_Orientation;
		DWORD System_Audio_SampleRate;
		DWORD System_DateModified;
		DWORD System_NamespaceCLSID;
		DWORD System_Keywords;
		DWORD System_GPS_DestLatitude;
		
		// see propkey.h for a list of supported VT_* types.
		system_size = Everything3_FindProperty(client,L"System.size"); // VT_UI8
		system_kind = Everything3_FindProperty(client,L"System.kind"); // VT_VECTOR|VT_LPWSTR
		system_keywords = Everything3_FindProperty(client,L"System.keywords");
		system_media_duration = Everything3_FindProperty(client,L"System.Media.Duration");
		bad_property = Everything3_FindProperty(client,L"abc");
		System_Image_HorizontalSize = Everything3_FindProperty(client,L"System.Image.HorizontalSize");
		System_FileExtension = Everything3_FindProperty(client,L"System.FileExtension"); // VT_LPWSTR / VT_EMPTY for folders
		System_Null = Everything3_FindProperty(client,L"System.Null"); // VT_NULL
		System_ParsingBindContext = Everything3_FindProperty(client,L"System.ParsingBindContext"); // VT_NULL / returned VT_EMPTY anyways, so tested VT_NULL Everything server side.
		System_Image_ResolutionUnit = Everything3_FindProperty(client,L"System.Image.ResolutionUnit"); // VT_I2
		System_Document_LineCount = Everything3_FindProperty(client,L"System.Document.LineCount"); // VT_I4
//		System_GPS_Altitude = Everything3_FindProperty(client,L"System.GPS.Altitude"); // VT_R8 => VT_R4 hack
		System_GPS_Altitude = Everything3_FindProperty(client,L"System.GPS.Altitude"); // VT_R8
//		system_size = Everything3_FindProperty(client,L"System.size"); VT_I8 HACK
//		system_size = Everything3_FindProperty(client,L"System.size"); VT_CY HACK
//		System_GPS_Altitude = Everything3_FindProperty(client,L"System.GPS.Altitude"); // VT_DATE HACK
		System_Title = Everything3_FindProperty(client,L"System.Title"); // VT_LPWSTR
//		System_Title = Everything3_FindProperty(client,L"System.Title"); // VT_LPSTR HACK
//		System_Title = Everything3_FindProperty(client,L"System.Title"); // VT_BSTR HACK
//		System_Document_LineCount = Everything3_FindProperty(client,L"System.Document.LineCount"); // VT_ERROR HACK
		System_Audio_IsVariableBitRate = Everything3_FindProperty(client,L"System.Audio.IsVariableBitRate"); // VT_BOOL
//		System_Document_LineCount = Everything3_FindProperty(client,L"System.Document.LineCount"); // VT_I1 HACK
		System_GPS_AltitudeRef = Everything3_FindProperty(client,L"System.GPS.AltitudeRef"); // VT_UI1
		System_Photo_Orientation = Everything3_FindProperty(client,L"System.Photo.Orientation"); // VT_UI2
		System_Audio_SampleRate = Everything3_FindProperty(client,L"System.Audio.SampleRate"); // VT_UI4
//		System_Document_LineCount = Everything3_FindProperty(client,L"System.Document.LineCount"); // VT_I8 HACK
//		System_Document_LineCount = Everything3_FindProperty(client,L"System.Document.LineCount"); // VT_INT HACK
//		System_Document_LineCount = Everything3_FindProperty(client,L"System.Document.LineCount"); // VT_UINT HACK
		System_DateModified = Everything3_FindProperty(client,L"System.DateModified"); // VT_FILETIME
//		System_Title = Everything3_FindProperty(client,L"System.Title"); // VT_BLOB HACK
		System_NamespaceCLSID = Everything3_FindProperty(client,L"System.NamespaceCLSID"); // VT_CLSID
		System_Keywords = Everything3_FindProperty(client,L"System.Keywords"); // VT_VECTOR|VT_LPWSTR
//		System_Keywords = Everything3_FindProperty(client,L"System.Keywords"); // VT_VECTOR|VT_LPSTR HACK
//		System_Keywords = Everything3_FindProperty(client,L"System.Keywords"); // VT_VECTOR|VT_BSTR HACK
		System_GPS_DestLatitude = Everything3_FindProperty(client,L"System.GPS.DestLatitude"); // VT_VECTOR|VT_R8

		
		printf("System.size %d\n",system_size);
		printf("System.kind %d\n",system_kind);
		printf("System.keywords %d\n",system_keywords);
		printf("System.Media.Duration %d\n",system_media_duration);
		printf("bad_property %d\n",bad_property);
		printf("System.Image.HorizontalSize %d\n",System_Image_HorizontalSize);
		printf("System.FileExtension %d\n",System_FileExtension);
		printf("System.Null %d\n",System_Null);
		printf("System.ParsingBindContext %d\n",System_ParsingBindContext);
		printf("System_Image_ResolutionUnit %d\n",System_Image_ResolutionUnit);
		printf("System_Document_LineCount %d\n",System_Document_LineCount);
		printf("System_GPS_Altitude %d\n",System_GPS_Altitude);
		printf("System_Title %d\n",System_Title);
		printf("System_Audio_IsVariableBitRate %d\n",System_Audio_IsVariableBitRate);
		printf("System_GPS_AltitudeRef %d\n",System_GPS_AltitudeRef);
		printf("System_Photo_Orientation %d\n",System_Photo_Orientation);
		printf("System_Audio_SampleRate %d\n",System_Audio_SampleRate);
		printf("System_DateModified %d\n",System_DateModified);
		printf("System_NamespaceCLSID %d\n",System_NamespaceCLSID);
		printf("System_Keywords %d\n",System_Keywords);
		printf("System_GPS_DestLatitude %d\n",System_GPS_DestLatitude);
		
		search_state = Everything3_CreateSearchState();
		if (search_state)
		{
//			Everything3_SetSearchTextUTF8(search_state,"C:\\Windows\\System32\\oobe\\background.bmp");
//			Everything3_SetSearchTextUTF8(search_state,"C:\\META\\MSG\\Sample MSG File.msg"); // Kind => email;communication
//			Everything3_SetSearchTextUTF8(search_state,"C:\\META\\PropertySystem title");
//			Everything3_SetSearchTextUTF8(search_state,"root:"); // VT_CLSID
			Everything3_SetSearchTextUTF8(search_state,"C:\\META\\PropertySystem\\");
			
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_PATH_AND_NAME);
			Everything3_AddSearchPropertyRequest(search_state,system_size);
			Everything3_AddSearchPropertyRequest(search_state,system_kind);
			Everything3_AddSearchPropertyRequest(search_state,system_keywords);
			Everything3_AddSearchPropertyRequest(search_state,system_media_duration);
			Everything3_AddSearchPropertyRequest(search_state,System_Image_HorizontalSize);
			Everything3_AddSearchPropertyRequest(search_state,System_FileExtension);
			Everything3_AddSearchPropertyRequest(search_state,System_Null); // can't find this property..
			Everything3_AddSearchPropertyRequest(search_state,System_ParsingBindContext); 
			Everything3_AddSearchPropertyRequest(search_state,System_Image_ResolutionUnit); 
			Everything3_AddSearchPropertyRequest(search_state,System_Document_LineCount); 
			Everything3_AddSearchPropertyRequest(search_state,System_GPS_Altitude); 
			Everything3_AddSearchPropertyRequest(search_state,System_Title); 
			Everything3_AddSearchPropertyRequest(search_state,System_Audio_IsVariableBitRate); 
			Everything3_AddSearchPropertyRequest(search_state,System_GPS_AltitudeRef); 
			Everything3_AddSearchPropertyRequest(search_state,System_Photo_Orientation); 
			Everything3_AddSearchPropertyRequest(search_state,System_Audio_SampleRate); 
			Everything3_AddSearchPropertyRequestFormatted(search_state,System_Audio_SampleRate); 
			Everything3_AddSearchPropertyRequest(search_state,System_DateModified); 
			Everything3_AddSearchPropertyRequest(search_state,System_NamespaceCLSID); 
			Everything3_AddSearchPropertyRequest(search_state,System_Keywords); 
			Everything3_AddSearchPropertyRequest(search_state,System_GPS_DestLatitude); 
			
			Everything3_SetSearchSort(search_state,EVERYTHING3_PROPERTY_ID_DATE_CREATED,TRUE); 
	
			result_list = Everything3_Search(client,search_state);
		
			if (result_list)
			{
				SIZE_T viewport_index;
				SIZE_T viewport_count;
				
				printf("Search OK\n");
				
				viewport_count = Everything3_GetResultListViewportCount(result_list);
					
				for(viewport_index=0;viewport_index<viewport_count;viewport_index++)
				{
					wchar_t filenameW[MAX_PATH];
					wchar_t wbuf[MAX_PATH];
					PROPVARIANT pv;

					PropVariantInit(&pv);

					Everything3_GetResultFullPathNameW(result_list,viewport_index,filenameW,MAX_PATH);
					printf("FilenameW: %S isfolder: %d\n",filenameW,Everything3_IsFolderResult(result_list,viewport_index));

					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,system_size,&pv))
					{
						if (pv.vt == VT_UI8)
						{
							printf("system.size: %I64u\n",pv.uhVal.QuadPart);
						}
						else
						if (pv.vt == VT_I8)
						{
							// hack server side to test VT_I8.
							printf("system.size: I8 HACK %I64d\n",pv.hVal.QuadPart);
						}
						else
						if (pv.vt == VT_CY)
						{
							// hack server side to test VT_CY
							printf("system.size: CY HACK %u\n",pv.cyVal.int64);
						}
						else
						{
							printf("system.size: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant system.size failed %08x\n",Everything3_GetLastError());
					}
					
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,system_kind,&pv))
					{
						if (pv.vt == (VT_VECTOR|VT_LPWSTR))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.calpwstr.cElems;vector_index++)
							{
								printf("system.kind (%u/%u): %S\n",vector_index+1,pv.calpwstr.cElems,pv.calpwstr.pElems[vector_index]);
							}
						}
						else
						{
							printf("system.kind: BAD vt %d\n",pv.vt);
						}
						
						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant system.kind failed %08x\n",Everything3_GetLastError());
					}
					
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_Image_HorizontalSize,&pv))
					{
						if (pv.vt == VT_UI4)
						{
							printf("System_Image_HorizontalSize: %u\n",pv.ulVal);
						}
						else
						{
							printf("System_Image_HorizontalSize: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_Image_HorizontalSize failed %08x\n",Everything3_GetLastError());
					}
						
					// System_FileExtension
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_FileExtension,&pv))
					{
						if (pv.vt == VT_LPWSTR)
						{
							printf("System_FileExtension: %S\n",pv.pwszVal);
						}
						else
						{
							printf("System_FileExtension: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_FileExtension failed %08x\n",Everything3_GetLastError());
					}

					// System_Null
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_Null,&pv))
					{
						if (pv.vt == VT_NULL)
						{
							printf("System_Null: NULL\n");
						}
						else
						{
							printf("System_Null: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_Null failed %08x\n",Everything3_GetLastError());
					}
				
					// System_ParsingBindContext
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_ParsingBindContext,&pv))
					{
						if (pv.vt == VT_NULL)
						{
							printf("System_ParsingBindContext: NULL\n");
						}
						else
						{
							printf("System_ParsingBindContext: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_ParsingBindContext failed %08x\n",Everything3_GetLastError());
					}
					
					// System_Image_ResolutionUnit
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_Image_ResolutionUnit,&pv))
					{
						if (pv.vt == VT_I2)
						{
							printf("System_Image_ResolutionUnit: %d\n",pv.iVal);
						}
						else
						{
							printf("System_Image_ResolutionUnit: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_Image_ResolutionUnit failed %08x\n",Everything3_GetLastError());
					}
					
					// System_Document_LineCount
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_Document_LineCount,&pv))
					{
						if (pv.vt == VT_I4)
						{
							printf("System_Document_LineCount: %d\n",pv.lVal);
						}
						else
						if (pv.vt == VT_ERROR)
						{
							printf("System_Document_LineCount: VT_ERROR HACK %d\n",pv.scode);
						}
						else
						if (pv.vt == VT_I1)
						{
							printf("System_Document_LineCount: VT_I1 HACK %d\n",pv.cVal);
						}
						else
						if (pv.vt == VT_I8)
						{
							printf("System_Document_LineCount: VT_I8 HACK %I64d\n",pv.hVal.QuadPart);
						}
						else
						if (pv.vt == VT_INT)
						{
							printf("System_Document_LineCount: VT_INT HACK %d\n",pv.intVal);
						}
						else
						if (pv.vt == VT_UINT)
						{
							printf("System_Document_LineCount: VT_UINT HACK %u\n",pv.uintVal);
						}
						else
						{
							printf("System_Document_LineCount: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_Document_LineCount failed %08x\n",Everything3_GetLastError());
					}
					
					// System_GPS_Altitude
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_GPS_Altitude,&pv))
					{
						if (pv.vt == VT_R8)
						{
							printf("System_GPS_Altitude: %f\n",pv.dblVal);
						}
						else
						if (pv.vt == VT_R4)
						{
							// modified everything server side to test VT_R4.
							printf("System_GPS_Altitude: HACK %f\n",pv.fltVal);
						}
						else
						if (pv.vt == VT_DATE)
						{
							printf("System_GPS_Altitude: DATE HACK %f\n",pv.date);
						}
						else
						{
							printf("System_GPS_Altitude: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_GPS_Altitude failed %08x\n",Everything3_GetLastError());
					}
					
					// System_Title
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_Title,&pv))
					{
						if (pv.vt == VT_LPWSTR)
						{
							printf("System_Title: %S\n",pv.pwszVal);
						}
						else
						if (pv.vt == VT_LPSTR)
						{
							printf("System_Title: VT_LPSTR HACK %s\n",pv.pszVal);
						}
						else
						if (pv.vt == VT_BSTR)
						{
							printf("System_Title: BSTR HACK %S\n",pv.pbstrVal);
						}
						else
						if (pv.vt == VT_BLOB)
						{
							printf("System_Title: BLOB HACK %u %u %S\n",pv.blob.cbSize,(wcslen((const wchar_t *)pv.blob.pBlobData)+1)*2,pv.blob.pBlobData);
						}
						else
						{
							printf("System_Title: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_Title failed %08x\n",Everything3_GetLastError());
					}
					
					// System_Audio_IsVariableBitRate
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_Audio_IsVariableBitRate,&pv))
					{
						if (pv.vt == VT_BOOL)
						{
							printf("System_Audio_IsVariableBitRate: %d\n",pv.boolVal);
						}
						else
						{
							printf("System_Audio_IsVariableBitRate: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_Audio_IsVariableBitRate failed %08x\n",Everything3_GetLastError());
					}
					
					// System_GPS_AltitudeRef
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_GPS_AltitudeRef,&pv))
					{
						if (pv.vt == VT_UI1)
						{
							printf("System_GPS_AltitudeRef: %u\n",pv.bVal);
						}
						else
						{
							printf("System_GPS_AltitudeRef: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_GPS_AltitudeRef failed %08x\n",Everything3_GetLastError());
					}
					
					// System_Photo_Orientation
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_Photo_Orientation,&pv))
					{
						if (pv.vt == VT_UI2)
						{
							printf("System_Photo_Orientation: %u\n",pv.uiVal);
						}
						else
						{
							printf("System_Photo_Orientation: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_Photo_Orientation failed %08x\n",Everything3_GetLastError());
					}
					
					// System_Audio_SampleRate
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_Audio_SampleRate,&pv))
					{
						if (pv.vt == VT_UI4)
						{
							printf("System_Audio_SampleRate: %u\n",pv.ulVal);
						}
						else
						{
							printf("System_Audio_SampleRate: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_Audio_SampleRate failed %08x\n",Everything3_GetLastError());
					}
					
					// System_Audio_SampleRate
					if (Everything3_GetResultPropertyTextFormatted(result_list,viewport_index,System_Audio_SampleRate,wbuf,MAX_PATH))
					{
						// printf breaks when wbuf contains LTR markers.
						wprintf(L"FORMATTED System_Audio_SampleRate: %ls\n",wbuf);
//						printf("FORMATTED System_Audio_SampleRate: %S\n",wbuf);
					}
					else
					if (GetLastError() == 0)
					{
						// empty text.
						wprintf(L"FORMATTED System_Audio_SampleRate: %ls\n",wbuf);
					}
					else
					{
						printf("Everything3_GetResultPropertyTextFormatted System_Audio_SampleRate failed %08x\n",Everything3_GetLastError());
					}
					
					// System_DateModified
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_DateModified,&pv))
					{
						if (pv.vt == VT_FILETIME)
						{
							printf("System_DateModified: 0x%016I64x\n",*(EVERYTHING3_UINT64 *)&pv.filetime);
						}
						else
						{
							printf("System_DateModified: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_DateModified failed %08x\n",Everything3_GetLastError());
					}
					
					// System_NamespaceCLSID
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_NamespaceCLSID,&pv))
					{
						if (pv.vt == VT_CLSID)
						{
							StringFromGUID2(pv.puuid,wbuf,MAX_PATH);
							wprintf(L"System_NamespaceCLSID: %ls\n",wbuf);
						}
						else
						{
							printf("System_NamespaceCLSID: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_NamespaceCLSID failed %08x\n",Everything3_GetLastError());
					}
					
					// System_Keywords
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_Keywords,&pv))
					{
						if (pv.vt == (VT_VECTOR|VT_LPWSTR))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.calpwstr.cElems;vector_index++)
							{
								printf("System_Keywords (%u/%u): %S\n",vector_index+1,pv.calpwstr.cElems,pv.calpwstr.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_BSTR))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.calpwstr.cElems;vector_index++)
							{
								printf("System_Keywords VT_VECTOR|VT_BSTR HACK (%u/%u): %S\n",vector_index+1,pv.calpwstr.cElems,pv.calpwstr.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_LPSTR))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.calpwstr.cElems;vector_index++)
							{
								printf("System_Keywords VT_VECTOR|VT_LPSTR HACK (%u/%u): %s\n",vector_index+1,pv.calpwstr.cElems,pv.calpwstr.pElems[vector_index]);
							}
						}
						else
						{
							printf("System_Keywords: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_Keywords failed %08x\n",Everything3_GetLastError());
					}
					
					// System_GPS_DestLatitude
					if (Everything3_GetResultPropertyPropVariant(result_list,viewport_index,System_GPS_DestLatitude,&pv))
					{
						if (pv.vt == (VT_VECTOR|VT_R8))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cadbl.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude (%u/%u): %f\n",vector_index+1,pv.cadbl.cElems,pv.cadbl.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_R4))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.caflt.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_R4 HACK (%u/%u): %f\n",vector_index+1,pv.caflt.cElems,pv.caflt.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_I2))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cai.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_I2 HACK (%u/%u): %d\n",vector_index+1,pv.cai.cElems,(int)pv.cai.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_I4))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cal.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_I4 HACK (%u/%u): %d\n",vector_index+1,pv.cal.cElems,pv.cal.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_INT))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cal.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_INT HACK (%u/%u): %d\n",vector_index+1,pv.cal.cElems,pv.cal.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_CY))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cacy.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_CY HACK (%u/%u): %I64d\n",vector_index+1,pv.cacy.cElems,pv.cacy.pElems[vector_index].int64);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_DATE))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cadate.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_DATE HACK (%u/%u): %f\n",vector_index+1,pv.cadate.cElems,pv.cadate.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_ERROR))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cascode.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_ERROR HACK (%u/%u): %u\n",vector_index+1,pv.cascode.cElems,pv.cascode.pElems[vector_index]);
							}
						}
						else	
						if (pv.vt == (VT_VECTOR|VT_BOOL))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cabool.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_BOOL HACK (%u/%u): %d\n",vector_index+1,pv.cabool.cElems,pv.cabool.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_I1))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cac.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_I1 HACK (%u/%u): %d\n",vector_index+1,pv.cac.cElems,pv.cac.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_UI1))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.caub.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_UI1 HACK (%u/%u): %d\n",vector_index+1,pv.caub.cElems,pv.caub.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_UI2))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.caui.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_UI2 HACK (%u/%u): %d\n",vector_index+1,pv.caui.cElems,pv.caui.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_UI4))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.caul.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_UI4 HACK (%u/%u): %u\n",vector_index+1,pv.caul.cElems,pv.caul.pElems[vector_index]);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_I8))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cah.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_I8 HACK (%u/%u): %I64d\n",vector_index+1,pv.cah.cElems,pv.cah.pElems[vector_index].QuadPart);
							}
						}
						else
						if (pv.vt == (VT_VECTOR|VT_UI8))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cauh.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_UI8 HACK (%u/%u): %I64u\n",vector_index+1,pv.cauh.cElems,pv.cauh.pElems[vector_index].QuadPart);
							}
						}
						else	
						if (pv.vt == (VT_VECTOR|VT_FILETIME))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cafiletime.cElems;vector_index++)
							{
								printf("System_GPS_DestLatitude VT_VECTOR|VT_FILETIME HACK (%u/%u): %I64u\n",vector_index+1,pv.cafiletime.cElems,*(EVERYTHING3_UINT64 *)&pv.cafiletime.pElems[vector_index]);
							}
						}
						else	
						if (pv.vt == (VT_VECTOR|VT_CLSID))
						{
							ULONG vector_index;
							for(vector_index=0;vector_index<pv.cauuid.cElems;vector_index++)
							{
								StringFromGUID2(&pv.cauuid.pElems[vector_index],wbuf,MAX_PATH);
								wprintf(L"System_GPS_DestLatitude VT_VECTOR|VT_CLSID HACK (%u/%u): %ls\n",vector_index+1,pv.cauuid.cElems,wbuf);
							}
						}
						else
						{
							printf("System_GPS_DestLatitude: BAD vt %d\n",pv.vt);
						}

						PropVariantClear(&pv);
					}
					else
					{
						printf("Everything3_GetResultPropertyPropVariant System_GPS_DestLatitude failed %08x\n",Everything3_GetLastError());
					}
					
					printf("\n");
					
					PropVariantClear(&pv);
				}
						
				Everything3_DestroyResultList(result_list);
			}
			else
			{
				printf("Search FAILED %08x\n",Everything3_GetLastError());
			}

			Everything3_DestroySearchState(search_state);
		}
		else
		{
			printf("Everything3_CreateSearchState FAILED %08x\n",Everything3_GetLastError());
		}
		
		// Disconnect and free the client.
		Everything3_DestroyClient(client);
	}
}

void test_Search(void)
{
	EVERYTHING3_CLIENT *client;

	client = test_Connect();
	if (client)
	{
		DWORD ipc_pipe_version;
		DWORD size_property;
		DWORD system_keywords_property;
		DWORD system_comment_property;
		SIZE_T  folder_result_count;
		SIZE_T  file_result_count;
		DWORD tickstart;
		SIZE_T viewport_offset;
		SIZE_T viewport_count;
		EVERYTHING3_UINT64 total_result_size;
		EVERYTHING3_RESULT_LIST *result_list;
		EVERYTHING3_SEARCH_STATE *search_state;
		
		printf("connected\n");
		
		ipc_pipe_version = Everything3_GetIPCPipeVersion(client);
		
		printf("version %d\n",ipc_pipe_version);
	
		size_property = Everything3_FindPropertyW(client,L"size");
		system_keywords_property = Everything3_FindPropertyW(client,L"system.keywords");
		system_comment_property = Everything3_FindPropertyW(client,L"system.comment");
		
		printf("size property id: %u\n",size_property);
		printf("system.keywords property id: %u\n",system_keywords_property);
		printf("system.comment property id: %u\n",system_comment_property);
		
		printf("IsDBLoaded: %d\n",Everything3_IsDBLoaded(client));
		

		// properties...
		/*
		{
			DWORD property_id;
			
			for(property_id=0;;property_id++)
			{
				BYTE name_utf8[256];
				wchar_t name_w[256];
				char name_a[256];
				
				if (!Everything3_GetPropertyNameUTF8(client,property_id,name_utf8,256))
				{
					break;
				}
				
				if (!Everything3_GetPropertyNameW(client,property_id,name_w,256))
				{
					break;
				}
				
				if (!Everything3_GetPropertyNameA(client,property_id,name_a,256))
				{
					break;
				}
				
				printf("Property %d: Name UTF8: %s\n",property_id,name_utf8);
				printf("Property %d: Name W: %S\n",property_id,name_w);
				printf("Property %d: Name A: %s\n",property_id,name_a);

				if (!Everything3_GetPropertyCanonicalNameUTF8(client,property_id,name_utf8,256))
				{
					break;
				}
				
				if (!Everything3_GetPropertyCanonicalNameW(client,property_id,name_w,256))
				{
					break;
				}
				
				if (!Everything3_GetPropertyCanonicalNameA(client,property_id,name_a,256))
				{
					break;
				}
				
				printf("Property %d: Canonical Name UTF8: %s\n",property_id,name_utf8);
				printf("Property %d: Canonical Name W: %S\n",property_id,name_w);
				printf("Property %d: Canonical Name A: %s\n",property_id,name_a);

				printf("Property %d: Type: %u\n",property_id,Everything3_GetPropertyType(client,property_id));
			}
		}
*/
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_NAME: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_NAME));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_PATH: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_PATH));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_SIZE: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_SIZE));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_EXTENSION: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_EXTENSION));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_TYPE: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_TYPE));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_DATE_MODIFIED: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_DATE_MODIFIED));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_DATE_CREATED: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_DATE_CREATED));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_DATE_ACCESSED: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_DATE_ACCESSED));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_ATTRIBUTES: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_ATTRIBUTES));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_DATE_RECENTLY_CHANGED: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_DATE_RECENTLY_CHANGED));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_RUN_COUNT: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_RUN_COUNT));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_DATE_RUN: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_DATE_RUN));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_FILE_LIST_NAME: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_FILE_LIST_NAME));
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_AUDIO_CHANNELS: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_AUDIO_CHANNELS));

		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_NAME: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_NAME));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_PATH: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_PATH));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_SIZE: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_SIZE));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_EXTENSION: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_EXTENSION));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_TYPE: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_TYPE));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_DATE_MODIFIED: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_DATE_MODIFIED));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_DATE_CREATED: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_DATE_CREATED));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_DATE_ACCESSED: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_DATE_ACCESSED));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_ATTRIBUTES: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_ATTRIBUTES));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_DATE_RECENTLY_CHANGED: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_DATE_RECENTLY_CHANGED));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_RUN_COUNT: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_RUN_COUNT));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_DATE_RUN: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_DATE_RUN));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_FILE_LIST_NAME: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_FILE_LIST_NAME));
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_AUDIO_CHANNELS: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_AUDIO_CHANNELS));

		Everything3_SetRunCountFromFilenameUTF8(client,"D:\\Downloads\\state-machine.jpg",123);
		printf("D:\\Downloads\\state-machine.jpg Run Count: %u\n",Everything3_GetRunCountFromFilenameUTF8(client,"D:\\Downloads\\state-machine.jpg"));
		Everything3_IncRunCountFromFilenameUTF8(client,"D:\\Downloads\\state-machine.jpg");
		printf("D:\\Downloads\\state-machine.jpg Run Count: %u\n",Everything3_GetRunCountFromFilenameUTF8(client,"D:\\Downloads\\state-machine.jpg"));

		Everything3_SetRunCountFromFilenameUTF8(client,"D:\\Downloads\\",321);
		printf("D:\\Downloads\\ Run Count: %u\n",Everything3_GetRunCountFromFilenameUTF8(client,"D:\\Downloads\\"));
		Everything3_IncRunCountFromFilenameUTF8(client,"D:\\Downloads\\");
		printf("D:\\Downloads\\ Run Count: %u\n",Everything3_GetRunCountFromFilenameUTF8(client,"D:\\Downloads\\"));

		printf("D:\\Downloads Folder Size: %I64u\n",Everything3_GetFolderSizeFromFilenameUTF8(client,"D:\\Downloads"));
		printf("D:\\Downloads\\ Folder Size: %I64u\n",Everything3_GetFolderSizeFromFilenameUTF8(client,"D:\\Downloads\\"));
		printf("d:\\downloads\\ Folder Size: %I64u\n",Everything3_GetFolderSizeFromFilenameUTF8(client,"d:\\downloads"));
		
		tickstart = GetTickCount();
		
		search_state = Everything3_CreateSearchState();
		if (search_state)
		{
			Everything3_SetSearchTextUTF8(search_state,"C:\\META\\--------10--------20--------30--------40--------50\\--------60--------70--------80--------90-------100\\-------110-------120-------130-------140-------150\\-------160-------170-------180-------190-------200\\-------210-------220-------230\\-------260-------270-------280-------290-------300");
//			Everything3_SetSearchTextUTF8(search_state,"A");
//			Everything3_SetSearchTextUTF8(search_state,"C:\\META\\DEEPPATH\\00\\01\\02\\03\\04\\05\\06\\07\\08\\09\\0A\\0B\\0C\\0D\\0E\\0F\\10\\11\\12\\13\\14\\15\\16\\17\\18\\19\\1A\\1B\\1C\\1D\\1E\\1F\\20\\21\\22\\23\\24\\25\\26\\27\\28\\29\\2A\\2B\\2C\\2D\\2E\\2F\\30\\31\\32\\33\\34\\35\\36\\37\\38\\39\\3A\\3B\\3C\\3D\\3E\\3F\\40\\41\\42\\43\\44\\45\\46\\47\\48\\49\\4A\\4B\\4C\\4D\\4E\\4F\\50\\51\\52\\53\\54\\55\\56\\57\\58\\59\\5A\\5B\\5C\\5D\\5E\\5F\\60\\61\\62\\63\\64\\65\\66\\67\\68\\69\\6A\\6B\\6C\\6D\\6E\\6F\\70\\71\\72\\73\\74\\75\\76\\77\\78\\79\\7A\\7B\\7C\\7D\\7E\\7F\\80\\81\\82\\83\\84\\85\\86\\87\\88\\89\\8A\\8B\\8C\\8D\\8E\\8F\\90\\91\\92\\93\\94\\95\\96\\97\\98\\99\\9A\\9B\\9C\\9D\\9E\\9F\\A0\\A1\\A2\\A3\\A4\\A5\\A6\\A7\\A8\\A9\\AA\\AB\\AC\\AD\\AE\\AF\\B0\\B1\\B2\\B3\\B4\\B5\\B6\\B7\\B8\\B9\\BA\\BB\\BC\\BD\\BE\\BF\\C0\\C1\\C2\\C3\\C4\\C5\\C6\\C7\\C8\\C9\\CA\\CB\\CC\\CD\\CE\\CF\\D0\\D1\\D2\\D3\\D4\\D5\\D6\\D7\\D8\\D9\\DA\\DB\\DC\\DD\\DE\\DF\\E0\\E1\\E2\\E3\\E4\\E5\\E6\\E7\\E8\\E9\\EA\\EB\\EC\\ED\\EE\\EF\\F0\\F1\\F2\\F3\\F4\\F5\\F6\\F7\\F8\\F9\\FA\\FB\\FC\\FD\\FE\\* folder:");
//			Everything3_SetSearchTextUTF8(search_state,"c:\\meta\\sdk folder:");
//			Everything3_SetSearchTextUTF8(search_state,"F:\\ sonic folder:");
//			Everything3_SetSearchTextUTF8(search_state,"Panasonic_DMC-FZ30"); // colorrepresentation:
//			Everything3_SetSearchTextUTF8(search_state,"Panasonic_DMC-FZ30 colorrepresentation:r"); // colorrepresentation:
//			Everything3_SetSearchTextUTF8(search_state,"Kodak_CX7530.jpg"); // latitude:
//			Everything3_SetSearchTextUTF8(search_state,"root:"); // test IsRoot

			// 3 byte test
//			Everything3_SetSearchTextUTF8(search_state,"\xE3\x81\x84\xE3\x82\x8D\xE3\x81\xAF");

			// 2 byte test
//			Everything3_SetSearchTextUTF8(search_state,"\xC3\xA9\xC3\xA0");
		
			// 4 byte test
			//Everything3_SetSearchTextUTF8(search_state,"\xF0\x9F\x91\x8D\xF0\x9F\x91\x8D");
			
			{
				SIZE_T search_len;
				
				search_len = Everything3_GetSearchTextUTF8(search_state,NULL,0);
				
				printf("Everything3_GetSearchTextUTF8 required size %Iu\n",search_len);
			}
			
			{
				SIZE_T search_len;
				
				search_len = Everything3_GetSearchTextW(search_state,NULL,0);
				
				printf("Everything3_GetSearchTextW required size %Iu\n",search_len);
			}
			
			{
				SIZE_T search_len;
				
				search_len = Everything3_GetSearchTextA(search_state,NULL,0);
				
				printf("Everything3_GetSearchTextA required size %Iu\n",search_len);
			}
			
			{
				EVERYTHING3_UTF8 search_text[256];
				SIZE_T search_len;
				
				search_len = Everything3_GetSearchTextUTF8(search_state,search_text,256);
				
				printf("Everything3_GetSearchTextUTF8: %Iu: %s\n",search_len,search_text);
			}
			
			{
				wchar_t search_text[256];
				SIZE_T search_len;
				
				search_len = Everything3_GetSearchTextW(search_state,search_text,256);
				
				printf("Everything3_GetSearchTextW: %Iu: %S\n",search_len,search_text);
			}
			
			{
				char search_text[256];
				SIZE_T search_len;
				
				search_len = Everything3_GetSearchTextA(search_state,search_text,256);
				
				printf("Everything3_GetSearchTextA: %Iu: %s\n",search_len,search_text);
			}
			
			Everything3_AddSearchSort(search_state,EVERYTHING3_PROPERTY_ID_DATE_MODIFIED,TRUE);
		
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_DESCENDANT_FOLDER_COUNT);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_NAME);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_PATH);
/*			
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_FULL_PATH);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_EXTENSION);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_TYPE);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_SIZE);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_DATE_MODIFIED);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_DATE_CREATED);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_DATE_ACCESSED);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_ATTRIBUTES);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_DATE_RECENTLY_CHANGED);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_RUN_COUNT);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_DATE_RUN);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_FILE_LIST_FILENAME);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_FILE_SIGNATURE);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_COLOR_REPRESENTATION);
			Everything3_AddSearchPropertyRequestFormatted(search_state,EVERYTHING3_PROPERTY_ID_COLOR_REPRESENTATION);
			Everything3_AddSearchPropertyRequestHighlighted(search_state,EVERYTHING3_PROPERTY_ID_COLOR_REPRESENTATION);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_BIT_DEPTH);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_FILE_ID);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_DIMENSIONS);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_FULL_PATH_LENGTH_IN_UTF8_BYTES);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_LATITUDE);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_CRC32);
			Everything3_AddSearchPropertyRequest(search_state,EVERYTHING3_PROPERTY_ID_SHA256);
			Everything3_SetSearchRequestTotalSize(search_state,TRUE);
*/			
			result_list = Everything3_Search(client,search_state);
			
			// test sort
/*
			if (result_list)
			{
				Everything3_DestroyResultList(result_list);
				
				Everything3_ClearSearchSorts(search_state);
				Everything3_AddSearchSort(search_state,EVERYTHING3_PROPERTY_ID_SIZE,TRUE);
				
				result_list = Everything3_Sort(client,search_state);
				if (result_list)
				{
					printf("Sort OK\n");
				}
			}
	*/
			if (result_list)
			{
				printf("Search OK\n");
				
				// test sort..

				for(;;)
				{
					folder_result_count = Everything3_GetResultListFolderCount(result_list);
					file_result_count = Everything3_GetResultListFileCount(result_list);
					total_result_size = Everything3_GetResultListTotalSize(result_list);
					
					viewport_offset = Everything3_GetResultListViewportOffset(result_list);
					viewport_count = Everything3_GetResultListViewportCount(result_list);
					
					// view port:
					
					printf("Found %u results in %u milliseconds\n",folder_result_count+file_result_count,GetTickCount() - tickstart);
					
					printf("folder result count %Iu\n",folder_result_count);
					printf("file result count %Iu\n",file_result_count);
					printf("total result size %I64u\n",total_result_size);

					printf("viewport offset %Iu\n",viewport_offset);
					printf("viewport count %Iu\n",viewport_count);

					{
						SIZE_T sort_count;
						SIZE_T sort_index;

						sort_count = Everything3_GetResultListSortCount(result_list);

						printf("sort count %Iu\n",sort_count);
						
						for(sort_index=0;sort_index<sort_count;sort_index++)
						{
							printf("sort %Iu: PropertyID: %u Ascending: %d\n",sort_index,Everything3_GetResultListSortPropertyId(result_list,sort_index),Everything3_GetResultListSortAscending(result_list,sort_index));
						}
					}

					{
						SIZE_T property_count;
						SIZE_T property_index;
						
						property_count = Everything3_GetResultListPropertyRequestCount(result_list);
						
						printf("property request count %Iu\n",property_count);

						for(property_index=0;property_index<property_count;property_index++)
						{
							printf("property request %2Iu: PropertyID: %3u ValueType: %2u\n",property_index,Everything3_GetResultListPropertyRequestPropertyId(result_list,property_index),Everything3_GetResultListPropertyRequestValueType(result_list,property_index));
						}					
					}
					
					{
						SIZE_T result_index;
						SIZE_T property_request_count;
						
						property_request_count = Everything3_GetResultListPropertyRequestCount(result_list);
						
						for(result_index=0;result_index<viewport_count;result_index++)
						{
							EVERYTHING3_UTF8 filenameUTF8[MAX_PATH];
							wchar_t filenameW[MAX_PATH];
							char filenameA[MAX_PATH];
							EVERYTHING3_UINT64 size;
//							LARGE_INTEGER size_large_integer;
							EVERYTHING3_UINT64 date_modified;
							EVERYTHING3_UINT64 date_created;
							EVERYTHING3_UINT64 date_accessed;
							DWORD attributes;
							EVERYTHING3_UINT64 date_recently_changed;
							DWORD run_count;
							EVERYTHING3_UINT64 date_run;
							DWORD file_signature;
							WORD color_representation;
							BYTE bit_depth;
							EVERYTHING3_UINT128 file_id;
							EVERYTHING3_DIMENSIONS dimensions;
							SIZE_T full_path_length_in_utf8_bytes;
							EVERYTHING3_INT32 latitude;
							BYTE crc32[4];
							SIZE_T crc32_size;
							BYTE sha256[32];
							SIZE_T sha256_size;

							printf("----------------------\n");
							printf("Is Folder: %d\n",Everything3_IsFolderResult(result_list,result_index));
							printf("Is Root: %d\n",Everything3_IsRootResult(result_list,result_index));
							
							Everything3_GetResultNameUTF8(result_list,result_index,filenameUTF8,MAX_PATH);
							printf("NameUTF8: %s\n",filenameUTF8);
							
							Everything3_GetResultNameW(result_list,result_index,filenameW,MAX_PATH);
							printf("NameW: %S\n",filenameW);
							
							Everything3_GetResultNameA(result_list,result_index,filenameA,MAX_PATH);
							printf("NameA: %s\n",filenameA);
							
							Everything3_GetResultPathUTF8(result_list,result_index,filenameUTF8,MAX_PATH);
							printf("PathUTF8: %s\n",filenameUTF8);
							
							Everything3_GetResultPathW(result_list,result_index,filenameW,MAX_PATH);
							printf("PathW: %S\n",filenameW);
							
							Everything3_GetResultPathA(result_list,result_index,filenameA,MAX_PATH);
							printf("PathA: %s\n",filenameA);
							
							Everything3_GetResultFullPathNameUTF8(result_list,result_index,filenameUTF8,MAX_PATH);
							printf("FullPathNameUTF8: %s\n",filenameUTF8);
							
							Everything3_GetResultFullPathNameW(result_list,result_index,filenameW,MAX_PATH);
							printf("FullPathNameW: %S\n",filenameW);
							
							Everything3_GetResultFullPathNameA(result_list,result_index,filenameA,MAX_PATH);
							printf("FullPathNameA: %s\n",filenameA);
							
							Everything3_GetResultExtensionUTF8(result_list,result_index,filenameUTF8,MAX_PATH);
							printf("ExtensionUTF8: %s\n",filenameUTF8);
							
							Everything3_GetResultExtensionW(result_list,result_index,filenameW,MAX_PATH);
							printf("ExtensionW: %S\n",filenameW);
							
							Everything3_GetResultExtensionA(result_list,result_index,filenameA,MAX_PATH);
							printf("ExtensionA: %s\n",filenameA);
							
							Everything3_GetResultTypeUTF8(result_list,result_index,filenameUTF8,MAX_PATH);
							printf("TypeUTF8: %s\n",filenameUTF8);
							
							Everything3_GetResultTypeW(result_list,result_index,filenameW,MAX_PATH);
							printf("TypeW: %S\n",filenameW);
							
							Everything3_GetResultTypeA(result_list,result_index,filenameA,MAX_PATH);
							printf("TypeA: %s\n",filenameA);
							
							size = Everything3_GetResultSize(result_list,result_index);
							printf("Size: %I64u\n",size);

//							Everything3_GetResultPropertyLargeInteger(result_list,result_index,EVERYTHING3_PROPERTY_ID_SIZE,&size_large_integer);
//							printf("Size Large Integer: %I64u\n",size_large_integer.QuadPart);

							date_modified = Everything3_GetResultDateModified(result_list,result_index);
							printf("Modified: %I64u\n",date_modified);
							
							date_created = Everything3_GetResultDateCreated(result_list,result_index);
							printf("Created: %I64u\n",date_created);
							
							date_accessed = Everything3_GetResultDateAccessed(result_list,result_index);
							printf("Accessed: %I64u\n",date_accessed);
							
							attributes = Everything3_GetResultAttributes(result_list,result_index);
							printf("Attributes: %08x\n",attributes);

							date_recently_changed = Everything3_GetResultDateRecentlyChanged(result_list,result_index);
							printf("Date Recently Changed: %I64u\n",date_recently_changed);

							run_count = Everything3_GetResultRunCount(result_list,result_index);
							printf("Run Count: %u\n",run_count);

							date_run = Everything3_GetResultDateRun(result_list,result_index);
							printf("Date Run: %I64u\n",date_run);

							Everything3_GetResultFilelistFilenameUTF8(result_list,result_index,filenameUTF8,MAX_PATH);
							printf("FilelistFilenameUTF8: %s\n",filenameUTF8);
							
							Everything3_GetResultFilelistFilenameW(result_list,result_index,filenameW,MAX_PATH);
							printf("FilelistFilenameW: %S\n",filenameW);
							
							Everything3_GetResultFilelistFilenameA(result_list,result_index,filenameA,MAX_PATH);
							printf("FilelistFilenameA: %s\n",filenameA);
							
							file_signature = Everything3_GetResultPropertyDWORD(result_list,result_index,EVERYTHING3_PROPERTY_ID_FILE_SIGNATURE);
							printf("file_signature: %u\n",file_signature);

							color_representation = Everything3_GetResultPropertyWORD(result_list,result_index,EVERYTHING3_PROPERTY_ID_COLOR_REPRESENTATION);
							printf("color_representation: %u\n",color_representation);
							
							Everything3_GetResultPropertyTextFormattedUTF8(result_list,result_index,EVERYTHING3_PROPERTY_ID_COLOR_REPRESENTATION,filenameUTF8,MAX_PATH);
							printf("color_representationFormatted: %s\n",filenameUTF8);
							
							Everything3_GetResultPropertyTextHighlightedUTF8(result_list,result_index,EVERYTHING3_PROPERTY_ID_COLOR_REPRESENTATION,filenameUTF8,MAX_PATH);
							printf("color_representationHighlighted: %s\n",filenameUTF8);
							
							bit_depth = Everything3_GetResultPropertyBYTE(result_list,result_index,EVERYTHING3_PROPERTY_ID_BIT_DEPTH);
							printf("bit_depth: %u\n",bit_depth);

							Everything3_GetResultPropertyUINT128(result_list,result_index,EVERYTHING3_PROPERTY_ID_FILE_ID,&file_id);
							printf("file id: %016I64x%016I64x\n",file_id.hi_uint64,file_id.lo_uint64);
							
							Everything3_GetResultPropertyDIMENSIONS(result_list,result_index,EVERYTHING3_PROPERTY_ID_DIMENSIONS,&dimensions);
							printf("dimensions: %ux%u\n",dimensions.width,dimensions.height);
							
							full_path_length_in_utf8_bytes = Everything3_GetResultPropertySIZE_T(result_list,result_index,EVERYTHING3_PROPERTY_ID_PATH_AND_NAME_LENGTH_IN_UTF8_BYTES);
							printf("path_and_name_length_in_utf8_bytes: %Iu\n",full_path_length_in_utf8_bytes);
							
							latitude = Everything3_GetResultPropertyINT32(result_list,result_index,EVERYTHING3_PROPERTY_ID_LATITUDE);
							printf("latitude: %d\n",latitude);
							printf("latitude: %f\n",(double)latitude/1000000.0);
							
							crc32_size = sizeof(crc32);
							Everything3_GetResultPropertyBlob(result_list,result_index,EVERYTHING3_PROPERTY_ID_CRC32,crc32,&crc32_size);
							printf("crc32: ");
							hexdump(crc32,crc32_size);
							printf("\n");
							
							sha256_size = sizeof(sha256);
							Everything3_GetResultPropertyBlob(result_list,result_index,EVERYTHING3_PROPERTY_ID_SHA256,sha256,&sha256_size);
							printf("sha256: ");
							hexdump(sha256,sha256_size);
							printf("\n");
						
							{
								SIZE_T descendant_folder_count;
								
								descendant_folder_count = Everything3_GetResultPropertySIZE_T(result_list,result_index,EVERYTHING3_PROPERTY_ID_DESCENDANT_FOLDER_COUNT);
								
								printf("descendant_folder_count: %p\n",descendant_folder_count);
							}
							
							/*
							{
								SIZE_T property_request_index;
								
								for(property_request_index=0;property_request_index<property_request_count;property_request_index++)
								{
									DWORD property_value_type;
									
									property_value_type = Everything3_GetResultListPropertyRequestValueType(result_list,property_request_index);
									
									printf("PropertyRequest %Iu: ValueType: %u\n",property_request_index,property_value_type);
								}
							}*/
						}
					}
						
					Everything3_DestroyResultList(result_list);
					result_list = NULL;

					/*
					for(;;)
					{
						if (Everything3_IsResultListChange(client))
						{
							result_list = Everything3_GetResults(client,search_state);
							if (!result_list)
							{
								printf("Everything3_GetResults FAILED %08x\n",Everything3_GetLastError());
							}
							break;
						}
						else
						{
							if (Everything3_GetLastError() != EVERYTHING3_OK)
							{
								printf("Everything3_IsResultListChange FAILED %08x\n",Everything3_GetLastError());
								
								break;
							}
						}
						
						Sleep(1000);
					}
					*/

					if (Everything3_WaitForResultListChange(client))
					{
						result_list = Everything3_GetResults(client,search_state);
						if (!result_list)
						{
							printf("Everything3_GetResults FAILED %08x\n",Everything3_GetLastError());
							break;
						}
					}
					else
					{
						printf("Everything3_WaitForResultListChange FAILED %08x\n",Everything3_GetLastError());
						
						break;
					}
					
					if (!result_list)
					{
						break;
					}
				}
			}
			else
			{
				printf("Search FAILED %08x\n",Everything3_GetLastError());
			}

			Everything3_DestroySearchState(search_state);
		}
		else
		{
			printf("Everything3_CreateSearchState FAILED %08x\n",Everything3_GetLastError());
		}
		
		// Sleep(10000);
		
		Everything3_DestroyClient(client);
	}
	else
	{
		printf("failed to connect %08x\n",Everything3_GetLastError());
	}
}

int main(int argc,EVERYTHING3_CHAR **argv)
{
	simple_example();	
//	test_viewport();
//	test_GetFileAttributesEx();
//	test_FindFirstFile();
	
	test_GetJournalInfo();
//	test_ReadJournal();
	test_IsPropertyRightAligned();
	test_IsPropertySortDescending();
	test_GetPropertyDefaultWidth();
	test_PropertySystem();
//	test_Search();

#ifdef _EVERYTHING3_DEBUG
	printf("mem_allocated: %Iu\n",_everything3_mem_allocated);
#endif

	return 0;
}
