#include "peas-gtk-osx.h"

#include <Cocoa/Cocoa.h>

void
peas_gtk_osx_show_uri (const gchar *uri)
{
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:uri]]];
}
