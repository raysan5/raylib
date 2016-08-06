/*******************************************************************************************
*
*   raylib [rlua] example - Lua file execution
*
*   NOTE: This example requires Lua library (http://luabinaries.sourceforge.net/download.html)
*
*   Compile example using:
*   gcc -o $(NAME_PART).exe $(FILE_NAME) $(RAYLIB_DIR)\raylib_icon          /
*       -I../src -I../src/external/lua/include -L../src/external/lua/lib    /
*       -lraylib -lglfw3 -lopengl32 -lopenal32 -llua53 -lgdi32 -std=c99
*
*   This example has been created using raylib 1.6 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define RLUA_IMPLEMENTATION
#include "rlua.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitLuaDevice();
    //--------------------------------------------------------------------------------------

    // ExecuteLuaFile("core_basic_window.lua");                 // OK!
		// ExecuteLuaFile("core_input_keys.lua");                   // OK!
		// ExecuteLuaFile("core_input_mouse.lua");                  // OK!
    // ExecuteLuaFile("core_mouse_wheel.lua");                  // OK!
    // ExecuteLuaFile("core_input_gamepad.lua");                // OK!
    // ExecuteLuaFile("core_random_values.lua");                // OK!
    // ExecuteLuaFile("core_color_select.lua");                 // OK!
    // ExecuteLuaFile("core_drop_files.lua");                   // ERROR: GetDroppedFiles()
    // ExecuteLuaFile("core_storage_values.lua");               // OK!
    // ExecuteLuaFile("core_gestures_detection.lua");           // OK!
    // ExecuteLuaFile("core_3d_mode.lua");                      // ERROR: Lua Error: attempt to index a number value - Begin3dMode()
    // ExecuteLuaFile("core_3d_picking.lua");                   // ERROR: Lua Error: attempt to index a number value
    // ExecuteLuaFile("core_3d_camera_free.lua");               // ERROR: Lua Error: attempt to index a number value
    // ExecuteLuaFile("core_3d_camera_first_person.lua");       // ERROR: Lua Error: attempt to index a number value
    // ExecuteLuaFile("core_2d_camera.lua");                    // ERROR: Lua Error: attempt to index a number value - Begin2dMode()
    // ExecuteLuaFile("core_world_screen.lua");                 // ERROR: Lua Error: attempt to index a number value
    // ExecuteLuaFile("core_oculus_rift.lua");                  // ERROR: Lua Error: attempt to index a number value
    // ExecuteLuaFile("shapes_logo_raylib.lua");                // OK!
    // ExecuteLuaFile("shapes_basic_shapes.lua");               // OK!
    // ExecuteLuaFile("shapes_colors_palette.lua");             // OK!
    // ExecuteLuaFile("shapes_logo_raylib_anim.lua");           // OK! NOTE: Use lua string.sub() instead of raylib SubText()
    // ExecuteLuaFile("textures_logo_raylib.lua");              // OK!
    // ExecuteLuaFile("textures_image_loading.lua");            // OK!
    // ExecuteLuaFile("textures_rectangle.lua");                // OK!
    // ExecuteLuaFile("textures_srcrec_dstrec.lua");            // OK!
    // ExecuteLuaFile("textures_to_image.lua");                 // OK!
    // ExecuteLuaFile("textures_raw_data.lua");                 // ERROR: Lua Error: attempt to index a number value
		// ExecuteLuaFile("textures_formats_loading.lua");          // ISSUE: texture.id not exposed to be checked
		// ExecuteLuaFile("textures_particles_trail_blending.lua"); // ERROR: Using struct
		// ExecuteLuaFile("textures_image_processing.lua");         // ERROR: GetImageData() --> UpdateTexture()
		// ExecuteLuaFile("textures_image_drawing.lua");            // OK!
		// ExecuteLuaFile("text_sprite_fonts.lua");                 // OK!
		// ExecuteLuaFile("text_bmfont_ttf.lua");                   // OK!
		// ExecuteLuaFile("text_rbmf_fonts.lua");                   // ERROR: Lua Error: attempt to index a nil value
		// ExecuteLuaFile("text_format_text.lua");                  // OK! NOTE: Use lua string.format() instead of raylib FormatText()
		// ExecuteLuaFile("text_font_select.lua");                  // OK!
		// ExecuteLuaFile("text_writing_anim.lua");                 // ERROR: SubText()
		// ExecuteLuaFile("models_geometric_shapes.lua");           // ERROR: Lua Error: attempt to index a number value - Begin3dMode(camera)
		// ExecuteLuaFile("models_box_collisions.lua");             //
		// ExecuteLuaFile("models_billboard.lua");                  //
		// ExecuteLuaFile("models_obj_loading.lua");                //
		// ExecuteLuaFile("models_heightmap.lua");                  //
    // ExecuteLuaFile("models_cubicmap.lua");                   //
		// ExecuteLuaFile("shaders_model_shader.lua");              //
		// ExecuteLuaFile("shaders_shapes_textures.lua");           //
    // ExecuteLuaFile("shaders_custom_uniform.lua");            //
    // ExecuteLuaFile("shaders_postprocessing.lua");            //
    // ExecuteLuaFile("shaders_standard_lighting.lua");         //
    // ExecuteLuaFile("audio_sound_loading.lua");               // OK!
    // ExecuteLuaFile("audio_music_stream.lua");                // OK!
    ExecuteLuaFile("audio_module_playing.lua");              // ERROR: Using struct
    ExecuteLuaFile("audio_raw_stream.lua");                  // ERROR: UpdateAudioStream()

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseLuaDevice();        // Close Lua device and free resources
    //--------------------------------------------------------------------------------------

	return 0;
}