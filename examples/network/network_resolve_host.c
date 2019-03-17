/*******************************************************************************************
 *
 *   raylib [network] example - Resolve host
 *
 *   Welcome to raylib!
 *
 *   To test examples, just press F6 and execute raylib_compile_execute script
 *   Note that compiled executable is placed in the same folder as .c file
 *
 *   You can find all basic examples on C:\raylib\raylib\examples folder or
 *   raylib official webpage: www.raylib.com
 *
 *   Enjoy using raylib. :)
 *
 *   This example has been created using raylib 2.0 (www.raylib.com)
 *   raylib is licensed under an unmodified zlib/libpng license (View raylib.h
 *for details)
 *
 *   Copyright (c) 2013-2016 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h" 

int main()
{
	// Setup
	int screenWidth  = 800;
	int screenHeight = 450;
	InitWindow(
		screenWidth, screenHeight, "raylib [network] example - ping pong");
	SetTargetFPS(60);

	SetTraceLogLevel(LOG_DEBUG);

	// Networking
	InitNetwork(); 
	 
    AddressInformation* addr = NULL;
	int count = ResolveHost(
        NULL, 
        "5210", 
        ADDRESS_TYPE_IPV4, 
        0                               // Uncomment any of these flags
        //  ADDRESS_INFO_NUMERICHOST    // or try them in conjunction to
        //  ADDRESS_INFO_NUMERICSERV    // specify custom behaviour from 
        //  ADDRESS_INFO_DNS_ONLY       // the function getaddrinfo()
        //  ADDRESS_INFO_ALL            //
        //  ADDRESS_INFO_FQDN           // e.g. ADDRESS_INFO_CANONNAME | ADDRESS_INFO_NUMERICSERV
        , 
        addr
    );   
	ResolveIP("8.8.8.8", NULL, NAME_INFO_DEFAULT, NULL, NULL);
	ResolveIP("2001:4860:4860::8888", "80", NAME_INFO_NUMERICSERV, NULL, NULL);

	// Main game loop
	while (!WindowShouldClose())
	{
		// Draw
		BeginDrawing();

		// Clear
		ClearBackground(RAYWHITE);

		// End draw
		EndDrawing();
	}

	// Cleanup
	CloseWindow();
	return 0;
}