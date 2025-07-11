
// Everything IPC test
// this tests the lib and the dll.

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define EVERYTHING3_USERAPI

// includes
#include <windows.h>

#include "Everything3.h"

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
	client = Everything3_ConnectW(L"meta");
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
	client = Everything3_ConnectW(L"meta");
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
	client = Everything3_ConnectW(L"1.5");
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
	client = Everything3_ConnectW(L"meta");
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
	client = Everything3_ConnectW(L"ntfs");
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
	client = Everything3_ConnectW(L"pipename%\\:check");
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
	client = Everything3_ConnectW(L"meta");
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

int main(int argc,EVERYTHING3_CHAR **argv)
{
	EVERYTHING3_CLIENT *client;

//	simple_example();	
//	test_viewport();
//	test_GetFileAttributesEx();
//	test_FindFirstFile();
//	return 0;
	
	client = Everything3_ConnectW(L"meta");
	
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
		printf("IsPropertyIndexed: EVERYTHING3_PROPERTY_ID_FILE_LIST_FILENAME: %d\n",Everything3_IsPropertyIndexed(client,EVERYTHING3_PROPERTY_ID_FILE_LIST_FILENAME));
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
		printf("IsPropertyFastSort: EVERYTHING3_PROPERTY_ID_FILE_LIST_FILENAME: %d\n",Everything3_IsPropertyFastSort(client,EVERYTHING3_PROPERTY_ID_FILE_LIST_FILENAME));
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
							printf("property request %2Iu: PropertyID: %3u ValueType: %2u Offset: %04Ix\n",property_index,Everything3_GetResultListPropertyRequestPropertyId(result_list,property_index),Everything3_GetResultListPropertyRequestValueType(result_list,property_index),Everything3_GetResultListPropertyRequestOffset(result_list,property_index));
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
							
							full_path_length_in_utf8_bytes = Everything3_GetResultPropertySIZE_T(result_list,result_index,EVERYTHING3_PROPERTY_ID_FULL_PATH_LENGTH_IN_UTF8_BYTES);
							printf("full_path_length_in_utf8_bytes: %Iu\n",full_path_length_in_utf8_bytes);
							
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

#ifdef _EVERYTHING3_DEBUG
	printf("mem_allocated: %Iu\n",_everything3_mem_allocated);
#endif

	return 0;
}
