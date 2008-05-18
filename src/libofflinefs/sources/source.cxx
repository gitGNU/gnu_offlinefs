#include "source.hxx"

using std::string;

Source::Source(File& f,int mode):f(f),size(0),mode(mode) {
   size=f.getattr<off_t>("offlinefs.size");
}
