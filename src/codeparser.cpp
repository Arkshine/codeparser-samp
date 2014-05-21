#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QProcess>
#include <QMessageBox>

#include <algorithm>
#include <sstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "codeparser.h"
#include "mainwindow.h"

CodeParser::CodeParser()
{

}

void CodeParser::combineFiles(QString path)
{
    QFileInfo pawncc_info(pawncc_file_path);
    QString include_dir = pawncc_info.path() + "/include/";

    QFile source(path);
    if(source.open(QIODevice::ReadOnly))
    {
        QTextStream in(&source);
        while(!in.atEnd())
        {
            QString line = in.readLine();
            if(line.contains("#include"))
            {
                std::string std_line = line.toUtf8().constData();

                boost::regex rx_include("[<](.*)[>]");
                boost::smatch match;

                if(boost::regex_search(std_line, match, rx_include))
                {
                    std::string path = match[1];

                    if(boost::algorithm::contains(path, ".."))
                    {
                        std::cout << "Skipping include (custom paths not supported yet)" << std::endl;
                        continue;
                    }

                    QString full_path = include_dir + QString::fromStdString(path) + ".inc";

                    // Do not parse the same file multiple times
                    if(combined_files.contains(full_path))
                        continue;

                    QFile subfile(full_path);
                    if(subfile.exists())
                    {
                        combined_files.insert(full_path);
                        combineFiles(full_path);
                    }
                }
            }
            else
            {
                if(line.contains("#define") || line.contains("forward") || line.contains("stock") || line.contains("native"))
                {
                    if(line.contains("//"))
                    {
                        line.remove(line.indexOf("//"), line.length());
                    }

                    this->combined_output.push_back(line.toUtf8().constData());
                }
                else
                {
                    //std::cout << "Skip: " << line.toUtf8().constData() << std::endl;
                }
            }

            this->lines_parsed++;
        }
    }
}

void CodeParser::parse()
{
    boost::regex rx_parse_define("[\\s]*(#define)\\s+([^\\s(%{<]+)(.*)");
    boost::regex rx_parse_function("(.*)(native|forward|stock)[\\s]+([a-zA-Z0-9_]+[\\s]*:[\\s]*|)([a-zA-Z0-9_@]+)[\\s]*[(](.*)[)](.*)");

    for(std::vector<std::string>::iterator line = combined_output.begin(); line != combined_output.end(); ++line)
    {
        if(boost::algorithm::contains(*line, "#define"))
        {
            boost::smatch match;
            if(boost::regex_match(*line, match, rx_parse_define))
            {
                this->definitions.insert(match[2]);
            }
            else
            {
                this->skipped_definitions.append(QString::fromStdString(*line) + "\n");
            }
        }
        else if(boost::algorithm::contains(*line, "native") || boost::algorithm::contains(*line, "forward") || boost::algorithm::contains(*line, "stock"))
        {
            boost::smatch match;
            if(boost::regex_match(*line, match, rx_parse_function))
            {
                std::string type = match[2];
                std::string return_type = match[3];
                std::string function = match[4];
                std::string params_raw = match[5];
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

                this->functions.insert({type, return_type, function, params});
            }
            else
            {
                this->skipped_functions.append(QString::fromStdString(*line) + "\n");
            }
        }
    }
}

void CodeParser::reset()
{
    this->lines_parsed = 0;
    this->combined_files.empty();
    this->combined_output.empty();
    this->definitions.empty();
    this->functions.empty();
}

QString CodeParser::getSublimeOutput()
{
    QString output;

    output = "{\n";
    output += "\t\"scope\": \"source.pawn - variable.other.pawn\",\n";
    output += "\t\"completions\": [\n";

    unsigned int f_count = 0;

    for(std::set<CodeParser::Function>::iterator it = this->functions.begin(); it != this->functions.end(); ++it)
    {
        std::stringstream line;
        line << "\t\t{ \"trigger\": \"" << it->function << "\", \"contents\": \"" << it->function << "(";

        unsigned int count = 0;
        for(std::vector<std::string>::const_iterator p_it = it->params.begin(); p_it != it->params.end(); ++p_it)
        {
            line << "${" << count + 1 << ":" << *p_it << "}";

            if(count != it->params.size() - 1)
                line << ", ";

            ++count;
        }

        line << ")\" }";

        if(!this->definitions.empty() || f_count != this->functions.size() - 1)
            line << ",\n";
        else
            line << "\n";

        output += QString::fromStdString(line.str());

        ++f_count;
    }

    unsigned int d_count = 0;

    for(std::set<std::string>::iterator it = this->definitions.begin(); it != this->definitions.end(); ++it)
    {
        std::stringstream line;
        line << "\t\t\"" << *it << "\"";

        if(d_count != this->definitions.size() - 1)
            line << ",\n";
        else
            line << "\n";

        output += QString::fromStdString(line.str());

        ++d_count;
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

    for(std::set<CodeParser::Function>::iterator it = this->functions.begin(); it != this->functions.end(); ++it)
    {
        std::stringstream line;
        line << "\t\t<KeyWord name=\""
             << it->function
             << "\" func=\"yes\">\n"
             << "\t\t\t<Overload retVal=\""
             << it->return_type
             << "\">\n";

        for(std::vector<std::string>::const_iterator p_it = it->params.begin(); p_it != it->params.end(); ++p_it)
        {
            line << "\t\t\t\t<Param name=\"" << boost::replace_all_copy(*p_it, "\\\"", "'") << "\" />\n";
        }

        line << "\t\t\t</Overload>\n\t\t</KeyWord>\n";

        pawn_str += QString::fromStdString(line.str());
        user_def_str.append(QString::fromStdString(it->function) + " ");
    }

    for(std::set<std::string>::iterator it = this->definitions.begin(); it != this->definitions.end(); ++it)
    {
        pawn_str += QString("\t\t<KeyWord name=\"%1\"/>\n").arg(QString::fromStdString(*it));
    }

    pawn_str.append("\t</AutoComplete>\n</NotepadPlus>");
    user_def_str.append("</Keywords>\n\t\t\t<Keywords name=\"Words2\">");

    for(std::set<std::string>::iterator it = this->definitions.begin(); it != this->definitions.end(); ++it)
    {
        user_def_str.append(QString::fromStdString(*it) + " ");
    }

    user_def_str.append("</Keywords>\n\t\t\t<Keywords name=\"Words3\">#assert #define #else #elseif #endif #endinput #error #file #if #include #line #pragma #tryinclude #undef #emit</Keywords>\n\t\t\t<Keywords name=\"Words4\">DB DBResult File Float Menu PlayerText3D Text Text3D _ anglemode assert bool break case char const continue default defined do else enum exit false filemode floatround_method for forward goto if library native new operator public return seek_whence sizeof sleep state static stock switch tagof true while</Keywords>\n\t\t</KeywordLists>\n\t\t<Styles>\n\t\t\t<WordsStyle name=\"DEFAULT\" styleID=\"11\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"FOLDEROPEN\" styleID=\"12\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"FOLDERCLOSE\" styleID=\"13\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"KEYWORD1\" styleID=\"5\" fgColor=\"000080\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"KEYWORD2\" styleID=\"6\" fgColor=\"800000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"KEYWORD3\" styleID=\"7\" fgColor=\"800000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"1\" />\n\t\t\t<WordsStyle name=\"KEYWORD4\" styleID=\"8\" fgColor=\"0000C0\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"1\" />\n\t\t\t<WordsStyle name=\"COMMENT\" styleID=\"1\" fgColor=\"008000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"COMMENT LINE\" styleID=\"2\" fgColor=\"008000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"NUMBER\" styleID=\"4\" fgColor=\"FF8000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"OPERATOR\" styleID=\"10\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"DELIMINER1\" styleID=\"14\" fgColor=\"808080\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"DELIMINER2\" styleID=\"15\" fgColor=\"808080\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t\t<WordsStyle name=\"DELIMINER3\" styleID=\"16\" fgColor=\"000000\" bgColor=\"FFFFFF\" fontName=\"\" fontStyle=\"0\" />\n\t\t</Styles>\n\t</UserLang>\n</NotepadPlus>");

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

std::set<CodeParser::Function> CodeParser::getFunctions()
{
    return this->functions;
}

std::set<std::string> CodeParser::getDefinitions()
{
    return this->definitions;
}

QString CodeParser::getSkippedFunctions()
{
    return this->skipped_functions;
}

QString CodeParser::getSkippedDefinitions()
{
    return this->skipped_definitions;
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

bool operator<(const CodeParser::Function &lhs, const CodeParser::Function &rhs)
{
    return boost::algorithm::to_lower_copy(lhs.function) < boost::algorithm::to_lower_copy(rhs.function);
}
