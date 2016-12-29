#/bin/sh
echo "Generate files..."
glib-compile-resources --c-name bmdc --generate-header --sourcedir=. --target=linux/genres.h .gresource.xml 
glib-compile-resources --c-name bmdc --generate-source --sourcedir=. --target=linux/genres.cc .gresource.xml 
echo "End..."
exit 0
