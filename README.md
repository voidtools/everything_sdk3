# everything_sdk3
C wrapper for Everything IPC over named pipes

Download
--------

https://www.voidtools.com/Everything-SDK3.zip
<br/><br/><br/>



The Everything 1.5 SDK is version 3 of the Everything SDK.

Version 2 is for Everything 1.4.

version 1 is for Everything 1.3.
<br/><br/>
Previous versions are compatible with Everything 1.5.
<br/><br/>
The Everything 1.5 SDK now uses named pipes.
<br/><br/>
The name of the pipe is:
<code>\\.\PIPE\Everything IPC</code><br/>
If Everything is running in an instance, the pipe name is: <code>\\.\PIPE\Everything IPC (instance-name)</code><br/>
Everything 1.5 alpha runs in a <code>1.5a</code> instance.<br/>
<br/><br/>
Pipe connections can be kept open if you wish to monitor result list changes.
<br/><br/>
Everything hosts 8 pipe servers by default.

pipe servers are recreated as soon as a client connects.

Note: it's possible (but unlikely) for 8 clients to connect at the same time and the 9th client will fail before a new pipe server is created.
<br/><br/>
To set the number of IPC pipe servers:

*    In Everything 1.5, from the Tools menu, click Options.
*    Click the Advanced tab on the left.
*    To the right of Show settings containing, search for:<br/>
     <code>menu</code>
*    Select: ipc_pipe_count
*    Set the value to: 8<br/>
     (where 8 is the number of IPC pipe servers)
*    Click OK.
<br/><br/><br/>


Example Usage
-------------

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
