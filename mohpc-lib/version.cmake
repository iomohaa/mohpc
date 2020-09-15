file(READ ${VERSION_FILE} VERSION_CONTENT)

string(REGEX MATCH "VERSION_BUILD[ ]*=[ ]*([0-9]+);" MY_MATCHES "${VERSION_CONTENT}")

# Get the build number in the first match
set(VERSION_BUILD ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_MAJOR[ ]*=[ ]*([0-9]+);" MY_MATCHES "${VERSION_CONTENT}")

# Get the build number in the first match
set(VERSION_MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_MINOR[ ]*=[ ]*([0-9]+);" MY_MATCHES "${VERSION_CONTENT}")

# Get the build number in the first match
set(VERSION_MINOR ${CMAKE_MATCH_1})

# Increment the build number
math(EXPR NEW_VERSION_BUILD "${VERSION_BUILD}+1")

# Replace the build number
string(REGEX REPLACE "VERSION_BUILD[ ]*=[ ]*([0-9]+);" "VERSION_BUILD = ${NEW_VERSION_BUILD};" NEW_VERSION_CONTENT "${VERSION_CONTENT}")

# Replace the version string
string(REGEX REPLACE
	"VERSION_STRING[[]] [ ]*=[ ]*(\"[A-Za-z0-9..]*\");"
	"VERSION_STRING[] = \"${VERSION_MAJOR}.${VERSION_MINOR}.${NEW_VERSION_BUILD}\";"
	NEW_VERSION_CONTENT "${NEW_VERSION_CONTENT}"
	)

# Replace the short version string
string(REGEX REPLACE
	"VERSION_SHORT_STRING[[]][ ]*=[ ]*(\"[A-Za-z0-9..]*\");"
	"VERSION_SHORT_STRING[] = \"${VERSION_MAJOR}.${VERSION_MINOR}\";"
	NEW_VERSION_CONTENT "${NEW_VERSION_CONTENT}"
	)

string(TIMESTAMP NOW "%b %d %Y")

# Set the build date and time
string(REGEX REPLACE
	"VERSION_DATE[[]][ ]*=[ ]*(\"[A-Za-z0-9 ]*\");"
	"VERSION_DATE[] = \"${NOW}\";"
	NEW_VERSION_CONTENT "${NEW_VERSION_CONTENT}"
	)

file(WRITE ${VERSION_FILE} "${NEW_VERSION_CONTENT}")
