#pragma once

#include "player.hpp"
#include "movieclip.hpp"

namespace openswf
{
    // forward declarations
    class Image;
    class FrameAction;
    class Stream;

    class Parser;
    struct Environment
    {
        Stream&         stream;
        Player&         player;

        MovieFrame      interrupted;
        MovieFrame      frame;
        MovieClip*      movie;
        TagHeader       tag;

        SWFHeader       header;

        Environment(Stream& stream, Player& player, const SWFHeader& header);
        bool advance();
    };

    class Parser
    {
    public:
        static bool         initialize();
        static bool         execute(Environment& env);
        static const char*  to_string(TagCode);

    protected:
        /// ----------------------------------------------------------------------------
        /// GENERIC CONTROL TAGS
        static void SetBackgroundColor(Environment&);
        static void Protect(Environment&) {}
        static void SymbolClass(Environment&) {}

        // the ExportAssets tag makes portions of a SWF file available for import by other SWF files.
        static void ExportAssets(Environment&);
        // the ImportAssets tag imports characters from another SWF file.
        static void ImportAssets(Environment&) {}
        static void ImportAssets2(Environment&) {}
        static void EnableDebugger(Environment&) {}
        static void EnableDebugger2(Environment&) {}
        static void ScriptLimits(Environment&);
        static void SetTabIndex(Environment&) {}
        static void FileAttributes(Environment&) {}
        static void Metadata(Environment&) {}

        /// ----------------------------------------------------------------------------
        /// GENERIC DEFINITION TAGS
        static void DefineScalingGrid(Environment&) {}
        static void DefineSceneAndFrameLabelData(Environment&) {}

        /// ----------------------------------------------------------------------------
        /// SPRITE DEFINITION AND RELATED CONTROL TAGS
        // the DefineSprite tag defines a sprite character.
        // it consists of a character ID and a frame count, followed by a series of control tags,
        // and terminated with an End tag.
        static void DefineSpriteHeader(Environment&);

        // the PlaceObject tag adds a character to the display list.
        static void PlaceObject(Environment&);

        // the PlaceObject2 tag can both add a character to the display list,
        // and modify the attributes of a character that is already on the display list. 
        // its changed slightly from SWF 4 to SWF 5. in SWF 5, clip actions were added.
        static void PlaceObject2(Environment&);

        // ??
        static void PlaceObject3(Environment&);

        // the RemoveObject tag removes the specified character (at the specified depth)
        // from the display list.
        static void RemoveObject(Environment&);

        // the RemoveObject2 tag removes the character at the specified depth from the
        // display list. 
        static void RemoveObject2(Environment&);

        // the FrameLabel tag gives the specified Name to the current frame.
        static void FrameLabel(Environment&);

        // DoAction instructs Flash Player to perform a list of actions when the current 
        // frame is complete. The actions are performed when the ShowFrame tag is encountered,
        // regardless of where in the frame the DoAction tag appears.
        static void DoAction(Environment&);

        // the ShowFrame tag instructs us to display the contents of the display list. 
        // the file is paused for the duration of a single frame.
        static void ShowFrame(Environment&);

        // the End tag indicates the end of file or sprite definition
        static void End(Environment&);


        /// ----------------------------------------------------------------------------
        /// SHAPE DEFINITION TAGS
        // shape in swf is similar to most vector formats which are defined by a list of
        // edges called a path.
        // the DefineShape tag defines a shape for later use by control tags such as PlaceObject.
        static void DefineShape(Environment&);

        // the DefineShape2 extends the capabilities of DefineShape with the ability to support
        // more than 255 styles in the style list and multiple style lists in a single shape.
        static void DefineShape2(Environment&);

        // the DefineShape3 extends the capabilities of DefineShape2 by extending all
        // of the RGB color fields to support RGBA with opacity information.
        static void DefineShape3(Environment&);

        // the DefineShape4 extends the capabilities of DefineShape3 by using a new line style
        // record in the shape. LINESTYLE2 allows new types of joins and caps as well as
        // scaling options and the ability to fill a stroke.
        static void DefineShape4(Environment&);

        // the DefineMorphShape tag defines the start and end states of a morph sequence. 
        static void DefineMorphShape(Environment&);

        // the DefineMorphShape2 tag extends the capabilities of DefineMorphShape by using
        // a new morph line style record in the morph shape. 
        static void DefineMorphShape2(Environment&);


        /// ----------------------------------------------------------------------------
        /// IMAGE DEFINITION TAGS
        // the DefineBits defines a bitmap character with JPEG compression.
        // It contains only the JPEG compressed image data (from the Frame Header onward).
        // A separate JPEGTables tag contains the JPEG encoding data used to encode this
        // image (the Tables/Misc segment).
        static void DefineBitsJPEG(Environment&) {}
        static void DefineBitsJPEGTable(Environment&) {}

        // the DefineBitsJPEG2 defines a bitmap character with JPEG compression.
        // It differs from DefineBits in that it contains both the JPEG encoding table
        // and the JPEG image data. This tag allows multiple JPEG images with differing
        // encoding tables to be defined within a single SWF file.
        static void DefineBitsJPEG2(Environment&);

        // the DefineBitsJPEG3 defines a bitmap character with JPEG compression.
        // This tag extends DefineBitsJPEG2, adding alpha channel (opacity) data.
        // Opacity/transparency information is not a standard feature in JPEG images,
        // so the alpha channel information is encoded separately from the JPEG data,
        // and compressed using the ZLIB standard for compression.
        static void DefineBitsJPEG3(Environment&);

        // ??
        static void DefineBitsJPEG4(Environment&) {}

        // the DefineBitsLossless defines a lossless bitmap character that contains
        // RGB bitmap data compressed with ZLIB. The data format used by the ZLIB
        // library is described by Request for Comments (RFCs) documents 1950 to 1952.
        static void DefineBitsLossless(Environment&);

        // the DefineBitsLossless2 extends DefineBitsLossless with support for opacity
        // (alpha values).
        static void DefineBitsLossless2(Environment&);
    };
}