#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QProcess>
#include <QMessageBox>

#include <algorithm>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "codeparser.h"
#include "mainwindow.h"

bool compare_function_names(const CodeParser::Function &lhs, const CodeParser::Function &rhs)
{
    return boost::algorithm::to_lower_copy(lhs.function) < boost::algorithm::to_lower_copy(rhs.function);
}

CodeParser::CodeParser()
{

}

QString CodeParser::runPreprocessor()
{
    QFileInfo pawncc_info(pawncc_file_path);
    QString include_dir = pawncc_info.path() + "/include/";

    QString temp_file = pawncc_info.path() + "/parser-temp.lst";

    QStringList params;
    params << source_file_path << "-l" << "-i" + include_dir << "-o" + temp_file;

    QProcess pawncc;
    pawncc.setProcessChannelMode(QProcess::MergedChannels);
    pawncc.setWorkingDirectory(pawncc_info.path());
    pawncc.start(pawncc_file_path, params);
    pawncc.waitForFinished(60000);

    QByteArray result;
    result = pawncc.readAllStandardOutput();

    if(result.contains("aborted"))
    {
        QMessageBox error;
        error.setWindowTitle("An error occurred while preprocessing the source file");
        error.setText(result);
        error.setIcon(QMessageBox::Warning);
        error.exec();

        temp_file = "";
    }

    return temp_file;
}

QString CodeParser::parse(QString path)
{
    this->lines_parsed = 0;
    this->functions.clear();

    QString output;

    QFile source(path);
    if(source.open(QIODevice::ReadOnly))
    {
        boost::regex rx_parse_function("(native|forward|stock)( |\\t)+([a-zA-Z]+[ \\t]*:[ \\t]*|)([a-zA-Z0-9_]+)(| |\\t)(\\()([a-zA-Z0-9 ,.\\[\\]=\\t&\\\"'-:_{}]*)(\\))([\\t ]*)");

        QTextStream in(&source);
        int line_number = 1;

        while(!in.atEnd())
        {
            QString line = in.readLine();

            if(line.contains("forward") || line.contains("stock") || line.contains("native"))
            {
                std::string std_line = line.toUtf8().constData();

                boost::smatch match;
                if(boost::regex_search(std_line, match, rx_parse_function))
                {
                    std::string type = match[1];
                    std::string return_type = match[3];
                    std::string function = match[4];
                    std::string params_raw = match[7];
                    std::vector<std::string> params;

                    std::stringstream ss(params_raw);
                    std::string param;

                    while(std::getline(ss, param, ','))
                    {
                        boost::erase_all(param, " ");
                        boost::erase_all(param, "\\t");
                        boost::erase_all(param, "{");
                        boost::erase_all(param, "}");
                        boost::replace_all(param, "\"", "\\\"");

                        params.push_back(param);
                    }

                    this->functions.push_back({type, return_type, function, params});
                }
                else
                {
                    // skipped += line;
                }
            }

            line_number++;
        }

        source.close();

        this->lines_parsed = line_number;
        std::sort(this->functions.begin(), this->functions.end(), compare_function_names);
    }

    return output;
}

QString CodeParser::getSublimeOutput()
{
    QString output;

    output = "{\n";
    output += "\t\"scope\": \"source.pawn - variable.other.pawn\",\n";
    output += "\t\"completions\": [\n";

    unsigned int f_count = 0;

    for(std::vector<CodeParser::Function>::iterator it = this->functions.begin(); it != this->functions.end(); ++it)
    {
        std::stringstream line;
        line << "\t\t{ \"trigger\": \"" << it->function << "\", \"contents\": \"" << it->function << "(";

        unsigned int count = 0;
        for(std::vector<std::string>::iterator p_it = it->params.begin(); p_it != it->params.end(); ++p_it)
        {
            line << "${" << count + 1 << ":" << *p_it << "}";

            if(count != it->params.size() - 1)
                line << ", ";

            ++count;
        }

        line << ")\" }";

        if(f_count != this->functions.size() - 1)
            line << ",\n";
        else
            line << "\n";

        output += QString::fromStdString(line.str());

        ++f_count;
    }

    output += "\t]\n";
    output += "}";

    return output;
}

QPair<QString, QString> CodeParser::getNotepadOutput()
{
    QString pawn_str;
    QString user_def_str;

    pawn_str = "<?xml version = \"1.0\" encoding=\"Windows-1252\" ?>\n<NotepadPlus>\n\t<AutoComplete language=\"PAWN\">\n\t\t<Environment ignoreCase=\"no\" startFunc=\"(\" stopFunc=\")\" paramSeparator=\",\" terminal=\";\" />\n\t\t<KeyWord name=\"#assert\" />\n\t\t<KeyWord name=\"#define\" />\n\t\t<KeyWord name=\"#else\" />\n\t\t<KeyWord name=\"#elseif\" />\n\t\t<KeyWord name=\"#emit\" />\n\t\t<KeyWord name=\"#endif\" />\n\t\t<KeyWord name=\"#endinput\" />\n\t\t<KeyWord name=\"#error\" />\n\t\t<KeyWord name=\"#file\" />\n\t\t<KeyWord name=\"#if\" />\n\t\t<KeyWord name=\"#include\" />\n\t\t<KeyWord name=\"#line\" />\n\t\t<KeyWord name=\"#pragma\" />\n\t\t<KeyWord name=\"#tryinclude\" />\n\t\t<KeyWord name=\"#undef\" />\n";
    user_def_str = "<NotepadPlus>\n\t<UserLang name = \"PAWN\" ext=\"pwn inc own\">\n\t\t<Settings>\n\t\t\t<Global caseIgnored=\"no\" escapeChar=\"\\\" />\n\t\t\t <TreatAsSymbol comment = \"yes\" commentLine=\"yes\" />\n\t\t\t<Prefix words1=\"no\" words2=\"no\" words3=\"no\" words4=\"no\" />\n\t\t</Settings>\n\t\t<KeywordLists>\n\t\t\t<Keywords name=\"Delimiters\">&quot;&apos;0&quot;&apos;0</Keywords>\n\t\t\t<Keywords name=\"Folder+\">{</Keywords>\n\t\t\t<Keywords name=\"Folder-\">}</Keywords>\n\t\t\t<Keywords name=\"Operators\">&apos; - ! &quot; % &amp; ( ) , : ; ? [ ] ^ { | } ~ + &lt; = &gt;</Keywords>\n\t\t\t<Keywords name=\"Comment\">1/* 2*/ 0//</Keywords>\n\t\t\t<Keywords name=\"Words1\">";

    for(std::vector<CodeParser::Function>::iterator it = this->functions.begin(); it != this->functions.end(); ++it)
    {
        std::stringstream line;
        line << "\t\t<KeyWord name=\""
             << it->function
             << "\" func=\"yes\">\n"
             << "\t\t\t<Overload retVal=\""
             << it->return_type
             << "\">\n";

        for(std::vector<std::string>::iterator p_it = it->params.begin(); p_it != it->params.end(); ++p_it)
        {
            boost::replace_all(*p_it, "\\\"", "'");
            line << "\t\t\t\t<Param name=\"" << *p_it << "\" />\n";
        }

        line << "\t\t\t</Overload>\n\t\t</KeyWord>\n";

        pawn_str += QString::fromStdString(line.str());
        user_def_str.append(QString::fromStdString(it->function) + " ");
    }

    pawn_str.append("\t</AutoComplete>\n</NotepadPlus>");
    user_def_str.append("</Keywords>\n\t\t\t<Keywords name=\"Words2\">SPECIAL_ACTION_NONE SPECIAL_ACTION_DUCK SPECIAL_ACTION_USEJETPACK SPECIAL_ACTION_ENTER_VEHICLE SPECIAL_ACTION_EXIT_VEHICLE SPECIAL_ACTION_DANCE1 SPECIAL_ACTION_DANCE2 SPECIAL_ACTION_DANCE3 SPECIAL_ACTION_DANCE4 SPECIAL_ACTION_HANDSUP SPECIAL_ACTION_USECELLPHONE SPECIAL_ACTION_SITTING SPECIAL_ACTION_STOPUSECELLPHONE SPECIAL_ACTION_DRINK_BEER SPECIAL_ACTION_SMOKE_CIGGY SPECIAL_ACTION_DRINK_WINE SPECIAL_ACTION_DRINK_SPRUNK FIGHT_STYLE_NORMAL FIGHT_STYLE_BOXING FIGHT_STYLE_KUNGFU FIGHT_STYLE_KNEEHEAD FIGHT_STYLE_GRABKICK FIGHT_STYLE_ELBOW WEAPONSKILL_PISTOL WEAPONSKILL_PISTOL_SILENCED WEAPONSKILL_DESERT_EAGLE WEAPONSKILL_SHOTGUN WEAPONSKILL_SAWNOFF_SHOTGUN WEAPONSKILL_SPAS12_SHOTGUN WEAPONSKILL_MICRO_UZI WEAPONSKILL_MP5 WEAPONSKILL_AK47 WEAPONSKILL_M4 WEAPONSKILL_SNIPERRIFLE WEAPONSTATE_UNKNOWN WEAPONSTATE_NO_BULLETS WEAPONSTATE_LAST_BULLET WEAPONSTATE_MORE_BULLETS WEAPONSTATE_RELOADING PLAYER_VARTYPE_NONE PLAYER_VARTYPE_INT PLAYER_VARTYPE_STRING PLAYER_VARTYPE_FLOAT MAX_CHATBUBBLE_LENGTH SPECTATE_MODE_NORMAL SPECTATE_MODE_FIXED SPECTATE_MODE_SIDE PLAYER_RECORDING_TYPE_NONE PLAYER_RECORDING_TYPE_DRIVER PLAYER_RECORDING_TYPE_ONFOOT _objects_included _samp_included PLAYER_RECORDING_TYPE_NONE PLAYER_RECORDING_TYPE_DRIVER PLAYER_RECORDING_TYPE_ONFOOT PLAYER_STATE_NONE PLAYER_STATE_ONFOOT PLAYER_STATE_DRIVER PLAYER_STATE_PASSENGER PLAYER_STATE_WASTED PLAYER_STATE_SPAWNED PLAYER_STATE_SPECTATING MAX_PLAYER_NAME MAX_PLAYERS MAX_VEHICLES INVALID_PLAYER_ID INVALID_VEHICLE_ID NO_TEAM MAX_OBJECTS INVALID_OBJECT_ID MAX_GANG_ZONES MAX_TEXT_DRAWS MAX_MENUS INVALID_MENU INVALID_TEXT_DRAW INVALID_GANG_ZONE WEAPON_BRASSKNUCKLE WEAPON_GOLFCLUB WEAPON_NITESTICK WEAPON_KNIFE WEAPON_BAT WEAPON_SHOVEL WEAPON_POOLSTICK WEAPON_KATANA WEAPON_CHAINSAW WEAPON_DILDO WEAPON_DILDO2 WEAPON_VIBRATOR WEAPON_VIBRATOR2 WEAPON_FLOWER WEAPON_CANE WEAPON_GRENADE WEAPON_TEARGAS WEAPON_MOLTOV WEAPON_COLT45 WEAPON_SILENCED WEAPON_DEAGLE WEAPON_SHOTGUN WEAPON_SAWEDOFF WEAPON_SHOTGSPA WEAPON_UZI WEAPON_MP5 WEAPON_AK47 WEAPON_M4 WEAPON_TEC9 WEAPON_RIFLE WEAPON_SNIPER WEAPON_ROCKETLAUNCHER WEAPON_HEATSEEKER WEAPON_FLAMETHROWER WEAPON_MINIGUN WEAPON_SATCHEL WEAPON_BOMB WEAPON_SPRAYCAN WEAPON_FIREEXTINGUISHER WEAPON_CAMERA WEAPON_PARACHUTE WEAPON_VEHICLE WEAPON_DROWN WEAPON_COLLISION KEY_ACTION KEY_CROUCH KEY_FIRE KEY_SPRINT KEY_SECONDARY_ATTACK KEY_JUMP KEY_LOOK_RIGHT KEY_HANDBRAKE KEY_LOOK_LEFT KEY_SUBMISSION KEY_LOOK_BEHIND KEY_WALK KEY_ANALOG_UP KEY_ANALOG_DOWN KEY_ANALOG_RIGHT KEY_ANALOG_LEFT KEY_UP KEY_DOWN KEY_LEFT KEY_RIGHT HTTP_GET HTTP_POST HTTP_HEAD HTTP_ERROR_BAD_HOST HTTP_ERROR_NO_SOCKET HTTP_ERROR_CANT_CONNECT HTTP_ERROR_CANT_WRITE HTTP_ERROR_CONTENT_TOO_BIG HTTP_ERROR_MALFORMED_RESPONSE _time_included _string_included _Float_included _file_included _datagram_included _core_included _vehicles_included CARMODTYPE_SPOILER CARMODTYPE_HOOD CARMODTYPE_ROOF CARMODTYPE_SIDESKIRT CARMODTYPE_LAMPS CARMODTYPE_NITRO CARMODTYPE_EXHAUST CARMODTYPE_WHEELS CARMODTYPE_STEREO CARMODTYPE_HYDRAULICS CARMODTYPE_FRONT_BUMPER CARMODTYPE_REAR_BUMPER CARMODTYPE_VENT_RIGHT CARMODTYPE_VENT_LEFT _sampdb_included _samp_included MAX_PLAYER_NAME MAX_PLAYERS MAX_VEHICLES INVALID_PLAYER_ID INVALID_VEHICLE_ID NO_TEAM MAX_OBJECTS INVALID_OBJECT_ID MAX_GANG_ZONES MAX_TEXT_DRAWS MAX_MENUS MAX_XT_GLOBAL MAX_3DTEXT_PLAYER MAX_PICKUPS INVALID_MENU INVALID_TEXT_DRAW INVALID_GANG_ZONE INVALID_3DTEXT_ID DIALOG_STYLE_MSGBOX DIALOG_STYLE_INPUT DIALOG_STYLE_LIST DIALOG_STYLE_PASSWORD PLAYER_STATE_NONE PLAYER_STATE_ONFOOT PLAYER_STATE_DRIVER PLAYER_STATE_PASSENGER PLAYER_STATE_EXIT_VEHICLE PLAYER_STATE_ENTER_VEHICLE_DRIVER PLAYER_STATE_ENTER_VEHICLE_PASSENGER PLAYER_STATE_WASTED PLAYER_STATE_SPAWNED PLAYER_STATE_SPECTATING PLAYER_MARKERS_MODE_OFF PLAYER_MARKERS_MODE_GLOBAL PLAYER_MARKERS_MODE_STREAMED WEAPON_BRASSKNUCKLE WEAPON_GOLFCLUB WEAPON_NITESTICK WEAPON_KNIFE WEAPON_BAT WEAPON_SHOVEL WEAPON_POOLSTICK WEAPON_KATANA WEAPON_CHAINSAW WEAPON_DILDO WEAPON_DILDO2 WEAPON_VIBRATOR WEAPON_VIBRATOR2 WEAPON_FLOWER WEAPON_CANE WEAPON_GRENADE WEAPON_TEARGAS WEAPON_MOLTOV WEAPON_COLT45 WEAPON_SILENCED WEAPON_DEAGLE WEAPON_SHOTGUN WEAPON_SAWEDOFF WEAPON_SHOTGSPA WEAPON_UZI WEAPON_MP5 WEAPON_AK47 WEAPON_M4 WEAPON_TEC9 WEAPON_RIFLE WEAPON_SNIPER WEAPON_ROCKETLAUNCHER WEAPON_HEATSEEKER WEAPON_FLAMETHROWER WEAPON_MINIGUN WEAPON_SATCHEL WEAPON_BOMB WEAPON_SPRAYCAN WEAPON_FIREEXTINGUISHER WEAPON_CAMERA WEAPON_PARACHUTE WEAPON_VEHICLE WEAPON_DROWN WEAPON_COLLISION KEY_ACTION KEY_CROUCH KEY_FIRE KEY_SPRINT KEY_SECONDARY_ATTACK KEY_JUMP KEY_LOOK_RIGHT KEY_HANDBRAKE KEY_LOOK_LEFT KEY_SUBMISSION KEY_LOOK_BEHIND KEY_WALK KEY_ANALOG_UP KEY_ANALOG_DOWN KEY_ANALOG_LEFT KEY_ANALOG_RIGHT KEY_UP KEY_DOWN KEY_LEFT KEY_RIGHT CLICK_SOURCE_SCOREBOARD floatround_round floatround_floor floatround_ceil floatround_tozero floatround_unbiased seek_start seek_current seek_end EOS cellbits cellmax cellmin charbits charmin charmax ucharmax __Pawn debug overlaysize radian degrees grades MAX_PLAYER_ATTACHED_OBJECTS VEHICLE_PARAMS_UNSET VEHICLE_PARAMS_OFF VEHICLE_PARAMS_ON OBJECT_MATERIAL_SIZE_32x32 OBJECT_MATERIAL_SIZE_64x32 OBJECT_MATERIAL_SIZE_64x64 OBJECT_MATERIAL_SIZE_128x32 OBJECT_MATERIAL_SIZE_128x64 OBJECT_MATERIAL_SIZE_128x128 OBJECT_MATERIAL_SIZE_256x32 OBJECT_MATERIAL_SIZE_256x64 OBJECT_MATERIAL_SIZE_256x128 OBJECT_MATERIAL_SIZE_256x256 OBJECT_MATERIAL_SIZE_512x64 OBJECT_MATERIAL_SIZE_512x128 OBJECT_MATERIAL_SIZE_512x256 OBJECT_MATERIAL_SIZE_512x512 OBJECT_MATERIAL_TEXT_ALIGN_LEFT OBJECT_MATERIAL_TEXT_ALIGN_CENTER OBJECT_MATERIAL_TEXT_ALIGN_RIGHT MAPICON_LOCAL MAPICON_GLOBAL MAPICON_LOCAL_CHECKPOINT MAPICON_GLOBAL_CHECKPOINT CAMERA_CUT CAMERA_MOVE MAX_PLAYER_TEXT_DRAWS MAX_3DTEXT_GLOBAL KEY_YES KEY_NO KEY_CTRL_BACK EDIT_RESPONSE_CANCEL EDIT_RESPONSE_FINAL EDIT_RESPONSE_UPDATE SELECT_OBJECT_GLOBAL_OBJECT SELECT_OBJECT_PLAYER_OBJECT VEHICLE_MODEL_INFO_SIZE VEHICLE_MODEL_INFO_FRONTSEAT VEHICLE_MODEL_INFO_REARSEAT VEHICLE_MODEL_INFO_PETROLCAP VEHICLE_MODEL_INFO_WHEELSFRONT VEHICLE_MODEL_INFO_WHEELSREAR VEHICLE_MODEL_INFO_WHEELSMID VEHICLE_MODEL_INFO_FRONT_BUMPER_Z VEHICLE_MODEL_INFO_REAR_BUMPER_Z io_read io_write io_readwrite io_append</Keywords>\n\t\t\t<Keywords name=\"Words3\">#assert #define #else #elseif #endif #endinput #error #file #if #include #line #pragma #tryinclude #undef #emit</Keywords>\n\t\t\t<Keywords name=\"Words4\">DB DBResult File Float Menu PlayerText3D Text Text3D _ anglemode assert bool break case char const continue default defined do else enum exit false filemode floatround_method for forward goto if library native new operator public return seek_whence sizeof sleep state static stock switch tagof true while</Keywords>\n\t\t</KeywordLists>\n\t\t<Styles>\n\t\t\t<WordsStyle name=\"DEFAULT\" styleID=\"11\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"FOLDEROPEN\" styleID=\"12\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"FOLDERCLOSE\" styleID=\"13\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"KEYWORD1\" styleID=\"5\" fgColor=\"000080\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"KEYWORD2\" styleID=\"6\" fgColor=\"800000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"KEYWORD3\" styleID=\"7\" fgColor=\"800000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"1\" />\n\t\t\t<WordsStyle name=\"KEYWORD4\" styleID=\"8\" fgColor=\"0000C0\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"1\" />\n\t\t\t<WordsStyle name=\"COMMENT\" styleID=\"1\" fgColor=\"008000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"COMMENT LINE\" styleID=\"2\" fgColor=\"008000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"NUMBER\" styleID=\"4\" fgColor=\"FF8000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"OPERATOR\" styleID=\"10\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"DELIMINER1\" styleID=\"14\" fgColor=\"808080\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"DELIMINER2\" styleID=\"15\" fgColor=\"808080\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"DELIMINER3\" styleID=\"16\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t</Styles>\n\t</UserLang>\n</NotepadPlus>");

    return QPair<QString, QString> (pawn_str, user_def_str);
}

// Setters
CodeParser *CodeParser::setSourceFile(QString path)
{
    this->source_file_path = path;
    return this;
}

CodeParser *CodeParser::setPawnccFile(QString path)
{
    this->pawncc_file_path = path;
    return this;
}

CodeParser *CodeParser::setSublimeOutputFile(QString path)
{
    this->sublime_output_path = path;
    return this;
}

CodeParser *CodeParser::setNotepadPawnFile(QString path)
{
    this->notepad_pawn_path = path;
    return this;
}

CodeParser *CodeParser::setNotepadUserDefFile(QString path)
{
    this->notepad_user_def_path = path;
    return this;
}

// Getters
QString CodeParser::getSourceFile()
{
    return this->source_file_path;
}

QString CodeParser::getPawnccFile()
{
    return this->pawncc_file_path;
}

QString CodeParser::getSublimeOutputFile()
{
    return this->sublime_output_path;
}

QString CodeParser::getNotepadPawnFile()
{
    return this->notepad_pawn_path;
}

QString CodeParser::getNotepadUserDefFile()
{
    return this->notepad_user_def_path;
}

std::vector<CodeParser::Function> CodeParser::getFunctions()
{
    return this->functions;
}

int CodeParser::getParsedLineCount()
{
    return this->lines_parsed;
}

QString CodeParser::limitStringLengthFromEnd(QString string, int limit)
{
    QString ret;

    if(string.length() > limit - 3)
        ret = "..." + string.right(limit - 3);
    else
        ret = string;

    return ret;
}
