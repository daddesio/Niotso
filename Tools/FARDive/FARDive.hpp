#include "version.hpp"

namespace Archive {
    extern wchar_t Path[1024], Filename[1024];
    
    extern bool IsOpen;
    extern bool IsModified;
    
    bool Add(const wchar_t * Path);
    bool Close();
    bool Open();
    bool PopulateEntries();
    bool Save();
    bool SaveAs();
    bool SetWorkspace();
}