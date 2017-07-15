$(document).ready(function() {
    
    // Init fancybox
    $('.fancybox').fancybox();

    var exampleName = [ 
        'core_basic_window',
        'core_input_keys',
        'core_input_mouse',
        'core_mouse_wheel',
        'core_input_gamepad',
        'core_random_values',
        'core_color_select',
        'core_drop_files',
        'core_storage_values',
        'core_gestures_detection',
        'core_3d_mode',
        'core_3d_picking',
        'core_3d_camera_free',
        'core_3d_camera_first_person',
        'core_2d_camera',
        'core_world_screen',
        'core_vr_simulator',
        'shapes_logo_raylib',
        'shapes_basic_shapes',
        'shapes_colors_palette',
        'shapes_logo_raylib_anim',
        'shapes_lines_bezier',
        'textures_logo_raylib',
        'textures_image_loading',
        'textures_rectangle',
        'textures_srcrec_dstrec',
        'textures_to_image',
        'textures_raw_data',
        'textures_image_processing',
        'textures_image_drawing',
        'text_sprite_fonts',
        'text_bmfont_ttf',
        'text_raylib_fonts',
        'text_format_text',
        'text_writing_anim',
        'text_ttf_loading',
        'text_bmfont_unordered',
        'text_input_box',
        'models_geometric_shapes',
        'models_box_collisions',
        'models_billboard',
        'models_obj_loading',
        'models_heightmap',
        'models_cubicmap',
        'models_mesh_picking',
        'shaders_model_shader',
        'shaders_shapes_textures',
        'shaders_custom_uniform',
        'shaders_postprocessing',
        'audio_sound_loading',
        'audio_music_stream',
        'audio_module_playing',
        'audio_raw_stream'];

    for (var i = 0; i < exampleName.length; i++)
    {
        var filterType = exampleName[i].substring(0, exampleName[i].indexOf("_"));
        var exampleBase = exampleName[i].slice(exampleName[i].indexOf('_') + 1);
        var exampleDesc = exampleBase.replace('_', ' ');

        $('#container').append(
            '<div class="mix f' + filterType + '">' +
            '<a class="fancybox fancybox.iframe" href="examples/web/' + filterType + '/' + 'loader.html?name=' + exampleName[i] + '" title="' + exampleDesc + '">' +
            '<img width="400" height="225" src="../examples/web/' + filterType + '/' + exampleName[i] + '.png"><div class="extext"><p>' + exampleDesc + '</p></div></a>' +
            '</div>');

        $('#container a .extext').hide();
    }

    // Instantiate MixItUp:
	$('#container').mixItUp();

    $("#container a").hover(
        function(){ $(this).find(".extext").show(); },
        function(){ $(this).find(".extext").hide(); });

/*
    $("#container a img").hover(
        function() { $(this).stop().animate({ opacity:0.6 }, 200, "easeOutQuad" ); },
        function() { $(this).stop().animate({ opacity:0 }, 200, "easeOutQuad" ); }
    )
*/
});