# everything_sdk3
C wrapper for Everything IPC over named pipes

<pre>
#include "Everything3.h"

void simple_example(void)
{
	EVERYTHING3_CLIENT *client;

	// connect to Everything.. Try the default unnamed instance first.
	client = Everything3_ConnectW(NULL);
	if (!client)
	{
		// connect to Everything.. Try the 1.5a instance.
		client = Everything3_ConnectW(L"1.5a");
	}

	if (client)
	{
		EVERYTHING3_SEARCH_STATE *search_state;

		// Connected..
		
		// Create an empty search state.
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
				for(result_index=0;result_index&lt;viewport_count;result_index++)
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
</pre>

https://www.voidtools.com/forum/viewtopic.php?t=15853
