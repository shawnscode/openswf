#pragma once

namespace openswf
{
    // displaying a frame of a SWF file is a three-stage process:
    // 1.   objects are defined with definition tags such as DefineShape, DefineSprite, and so on. 
    //      each object is given a unique ID called a character, 
    //      and is stored in a repository called the dictionary.
    // 2.   selected characters are copied from the dictionary and placed on the display list, 
    //      which is the list of the characters that will be displayed in the next frame.
    // 3.   once complete, the contents of the display list are rendered to the screen with ShowFrame.

    // a depth value is assigned to each character on the display list. 
    // the depth determines the stacking order of the character. Characters with lower 
    // depth values are displayed underneath characters with higher depth values. 
    // a character with a depth value of 1 is displayed at the bottom of the stack. 
    // a character can appear more than once in the display list, but at different depths. 
    // only one character can be at any given depth.

    // in SWF 3 and later versions, the display list is a hierarchical list 
    // where an element on the display can have a list of child elements.
    class display_list
    {

    };
}