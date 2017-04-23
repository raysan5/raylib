$(document).ready(function() {
    
    // Init fancybox
    $('.fancybox').fancybox();

    var exampleName = [ 
        'sample_arkanoid',
        'sample_missile_commander',
        'sample_gorilas',
        'sample_snake',
        'sample_asteroids',
        'sample_asteroids_survival',
        'sample_pang',
        'sample_space_invaders',
        'sample_floppy',
        'sample_tetris',
        'game_drturtle',
        'game_just_do',
        'game_skully_escape',
        'game_koala_seasons',
        'game_light_my_ritual',
        'game_wave_collector',
        'user_raymario',    
        'user_crazyball',
        'user_tictactoe',
        'user_super_red_rope',
        'user_taptojamp',    
        'user_funwithsat'];

    var exampleDesc = [
        'arkanoid',
        'missile commander',
        'gorilas',
        'snake',
        'asteroids',
        'asteroids survival',
        'pang',
        'space invaders',
        'floppy',
        'tetris',
        'Dr Turtle & Mr Gamera',
        'JUST DO',
        'Skully Escape',
        'Koala Seasons',
        'LIGHT MY RITUAL',
        'Wave Collector',
        'RayMario (Victor Fisac)',    
        'CrazyBall (Victor Fisac)',
        'Tic-Tac-Toe (LelixSuper)',
        'Super Red Rope Puzzles (Kurnic)',
        'TapToJAmp (MarcMDE)',    
        'FunWithSATCollisions (MarcMDE)'];

    for (var i = 0; i < exampleName.length; i++)
    {
        var filterType = exampleName[i].substring(0, exampleName[i].indexOf("_"));
        
        var linkTag;
        
        if (filterType == 'sample') linkTag = '<a class="fancybox fancybox.iframe" href="games/loader.html?name=' + exampleName[i] + '" title="' + exampleDesc[i] + '">';
        else if (filterType == 'game') linkTag = '<a target="_blank" href="games/' + exampleName[i].substring(5) + '.html" title="' + exampleDesc[i] + '">';
        else if (filterType == 'user')
        {
            switch (exampleName[i])
            {
                case 'user_raymario': linkTag = '<a target="_blank" href="https://github.com/victorfisac/RayMario" title="' + exampleDesc[i] + '">'; break;
                case 'user_crazyball': linkTag = '<a target="_blank" href="https://github.com/victorfisac/CrazyBall" title="' + exampleDesc[i] + '">'; break;
                case 'user_tictactoe': linkTag = '<a target="_blank" href="https://github.com/LelixSuper/TicTacToe" title="' + exampleDesc[i] + '">'; break;
                case 'user_super_red_rope': linkTag = '<a target="_blank" href="https://kurnic.itch.io/super-red-rope-puzzles" title="' + exampleDesc[i] + '">'; break;
                case 'user_taptojamp': linkTag = '<a target="_blank" href="http://marcmde.github.io/TapToJAmp_v2_0/" title="' + exampleDesc[i] + '">'; break;
                case 'user_funwithsat': linkTag = '<a target="_blank" href="http://marcmde.github.io/FunWithSATCollisions_v_1_0_Browser/" title="' + exampleDesc[i] + '">'; break;
                default: break;
            }
            /*
            https://github.com/MarcMDE/TapToJump    //http://marcmde.github.io/TapToJump_v1_0_Browser/
            https://github.com/MarcMDE/TapToJAmp    //http://marcmde.github.io/TapToJAmp_v2_0/
            https://github.com/MarcMDE/FunWithSATCollisions //http://marcmde.github.io/FunWithSATCollisions_v_1_0_Browser/
            https://kurnic.wordpress.com/portfolio/flappy-jetpack/
            */
        }
        
        $('#container').append(
            '<div class="mix f' + filterType + '">' + linkTag +
            '<img width="400" height="225" src="../games/img/' + exampleName[i] + '.png"><div class="extext"><p>' + exampleDesc[i] + '</p></div></a>' +
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