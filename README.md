#Code parser for Pawn
#### Generate auto-completion files for Sublime Text and Notepad++

This is a simple program written in Qt, which allows you to generate auto-completion files for Pawn language for Notepad++ and Sublime Text. The generated files will include every function in your gamemode or filterscript and all the functions from the files included in the main script.

![alt tag](https://dl.dropboxusercontent.com/s/2bg8ej2mjsb2z2w/CodeParser2.png)

## Usage

###General

1. Click browse and select the file based on which the auto-completion files will be generated. This is usually your gamemode or filterscript.

2. Click the second browse button, and locate your pawn compiler `pawncc.exe`. This is usually located in the `pawno/` directory.

3. Complete the steps under Sublime text or Notepad++ sub heading

4. Click the Generate autocomplete files button

5. You can save the selected file paths by clicking "Save configuration"

#### Sublime text
* Choose the output location for the auto-complete file. This is usually located in `%APPDATA%\Sublime Text 3\Packages\User` directory. The extension of the file should be `.sublime-completions`

#### Notepad++

* Choose the output locations for both `pawn.xml` and `userDefineLang.xml`. The userDefineLang.xml is usually located in `%APPDATA%\Notepad++\`, and pawn.xml is usually in `C:\Program Files (x86)\Notepad++\plugins\APIs`

* It is likely that you do not have write permissions to `Program Files` directory. If this is the case, you can either run the program with escalated privileges, or choose another location and copy the file there later.

* If you decide to override the `userDefineLang.xml` in `%APPDATA` directory, you will lose other language definitions if you have previously defined them in the same file. The files are not merged, they are overridden.