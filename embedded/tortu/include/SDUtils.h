#include <SDFS.h>

void listDirectoryContents(const char *dirname)
{
    // Recursive directory traversal using a lambda function
    auto listDir = [](const char *dirname, auto &listDirRef) -> void
    {
        Dir dir = SDFS.openDir(dirname);
        while (dir.next())
        {
            String name = dir.fileName();
            // Skip hidden directories and files
            if (name.startsWith("."))
            {
                continue;
            }

            String fullPath = String(dirname) + name;
            if (dir.isDirectory())
            {
                Serial.print("Directory: ");
                Serial.println(fullPath);
                // Ensure the path ends with a slash
                if (fullPath.charAt(fullPath.length() - 1) != '/')
                {
                    fullPath += "/";
                }
                listDirRef(fullPath.c_str(), listDirRef);
            }
            else
            {
                Serial.print("File: ");
                Serial.print(fullPath);
                File f = dir.openFile("r");
                Serial.print(" - Size: ");
                Serial.println(f.size());
            }
        }
    };

    // Start the recursive listing from the given directory
    listDir(dirname, listDir);
}