

DIR=`dirname $0`
. $DIR/tsf4gversion.var

if test $tsf4g_patch != X ; then
	tsf4g_version=${tsf4g_major}.${tsf4g_minor}.${tsf4g_patch}
	tsf4g_type=Release
else
	tsf4g_version=${tsf4g_major}.${tsf4g_minor}
	tsf4g_type=Devel
fi

tsf4g_string="${tsf4g_package} ${tsf4g_version}-${tsf4g_type}"

echo TSF4G_PACKAGE=\"${tsf4g_package}\"
echo TSF4G_MAJOR=$tsf4g_major
echo TSF4G_MINOR=$tsf4g_minor
echo TSF4G_PATCH=$tsf4g_patch
echo TSF4G_VERSION=$tsf4g_version
echo TSF4GL_TYPE=$tsf4g_type
echo TSF4G_STRING=\"${tsf4g_string}\"

