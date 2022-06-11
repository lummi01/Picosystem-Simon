#include "simon.hpp"
#include "assets.hpp"

using namespace blit;

Font font(asset_font8x8);

short state = 0;
int delay;
short best = 0;

struct Player
{
    short number;
    short level;
};

struct Simon
{
    short mem[99];
    short number;
    short rot;
};

struct PushButton
{
    short blend;
    short dblend;
    short number;
};

Player p;
Simon s;
PushButton bp;

Surface* background = Surface::load(asset_background);

void render_button()
{

}

///////////////////////////////////////////////////////////////////////////
//
// init()
//
// setup your game here
//
void init() 
{
    set_screen_mode(ScreenMode::lores);
 
    screen.sprites = Surface::load(asset_spritesheet);

    if (read_save(best))
    {
    }
    else 
    {
        best = 0;
    }
}

///////////////////////////////////////////////////////////////////////////
//
// render(time)
//
// This function is called to perform rendering of the game. time is the 
// amount if milliseconds elapsed since the start of your game
//
void render(uint32_t time) 
{
    screen.mask = nullptr;

    screen.blit(background, Rect(0, 0, 120, 120), Point(0, 0));

    if (bp.number > 0)
    {
        screen.alpha = 255 - bp.blend;

        if (bp.number == 1)
        {
            screen.sprite(Rect(0, 0, 10, 6), Point(20, -1));
        }
        else if (bp.number == 2)
        {
            screen.sprite(Rect(0, 0, 6, 10), Point(73, 20), SpriteTransform::R90);
        }
        else if (bp.number == 3)
        {
            screen.sprite(Rect(0, 0, 10, 6), Point(20, 73), SpriteTransform::R180);
        }
        else if (bp.number == 4)
        {
            screen.sprite(Rect(0, 0, 6, 10), Point(-1, 20), SpriteTransform::R270);
        }
    }

    screen.alpha = 255;
    screen.pen = Pen(255, 255, 255);
    screen.text(std::to_string(p.level) + "/" + std::to_string(best), font, Point(119, 2), true, TextAlign::top_right);

    if (state == 0)
    {
        screen.alpha = 128;
        screen.pen = Pen(128, 128, 128);        
        screen.rectangle(Rect(0, 88, 120, 12));
        screen.alpha = 255;
        screen.pen = Pen(0, 0, 0);
        screen.text("PRESS A TO START", font, Point(17, 91), true);
        screen.pen = Pen(255, 255, 255);
        screen.text("PRESS A TO START", font, Point(17, 90), true);
    }    
}

///////////////////////////////////////////////////////////////////////////
//
// update(time)
//
// This is called to update your game state. time is the 
// amount if milliseconds elapsed since the start of your game
//
void update(uint32_t time) 
{
    if (bp.number == 0)
    {
        if (state == 0 && delay == 0)
        {
            s.rot++;
            if (s.rot > 4)
            {
                s.rot = 1;
            }
            bp.number = s.rot;
        }
        else if (state == 1 && delay == 0) // Player answer
        {
            if (buttons.pressed & Button::A || buttons.pressed & Button::DPAD_RIGHT)
            {
                bp.number = 2;
            }
            else if (buttons.pressed & Button::B || buttons.pressed & Button::DPAD_DOWN)
            {
                bp.number = 3;
            }
            else if (buttons.pressed & Button::X || buttons.pressed & Button::DPAD_UP)
            {
                bp.number = 1;
            }
            else if (buttons.pressed & Button::Y || buttons.pressed & Button::DPAD_LEFT)
            {
                bp.number = 4;
            }
            if (bp.number > 0)
            {
                channels[1].trigger_attack();
                if (bp.number == s.mem[s.number])
                {
                    channels[1].frequency = 500 + (150 * bp.number);
                    s.number++;
                    if (s.mem[s.number] == 0)
                    {
                        p.level++;
                        if (p.level > best)
                        {
                            best = p.level;
                        }
                        s.mem[s.number] = 1 + (blit::random() % 4);
                        s.number = 0;
                        state = 2;
                        delay = 20;
                    }
                }
                else 
                {
                    channels[1].waveforms   = Waveform::SQUARE;
                    channels[1].frequency   = 100;
                    channels[1].decay_ms    = 500;
                    state = 3;
                    delay = 100;
                }
            }
        }
        else if(state == 2  && delay == 0) // Simon say
        {
            if (s.mem[s.number] > 0)
            {
                bp.number = s.mem[s.number];
                channels[1].trigger_attack();
                channels[1].frequency = 500 + (150 * bp.number);
                s.number++;
                delay = 2;
            }
            else
            {
                s.number = 0;
                state = 1;
            }
        }
        else if(state == 3  && delay == 0) // wrong answer
        {
            write_save(best);
            state = 0;
            delay = 10;
        }
        else
        {
            delay--;
        }
    }
    else
    {
        bp.blend += (bp.dblend - 36);
        bp.dblend += 3;
        if (bp.dblend > 72)
        {
            bp.blend = 255;
            bp.dblend = 0;
            bp.number = 0;
            
            channels[1].trigger_release();
        }
        if (state == 0 && buttons & Button::A)
        {
            bp.blend = 255;
            bp.dblend = 0;
            bp.number = 0;

            p.level = 0;

            s.number = 0;
            s.mem[0] = s.rot;
            for (int i = 1; i < 99; i++)
            {
                s.mem[i] = 0;
            }
            
            channels[1].waveforms   = Waveform::SQUARE;
            channels[1].frequency   = 0;
            channels[1].attack_ms   = 5;
            channels[1].decay_ms    = 200;
            channels[1].sustain     = 0;
            channels[1].release_ms  = 5;

            state = 2;
            delay = 10;
        }
    }
}

