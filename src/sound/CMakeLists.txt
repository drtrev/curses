INCLUDE_DIRECTORIES( ../../include /home/csunix/trev/shoot/include ) # need both of these again, even the absolute address
ADD_LIBRARY( sound sound dev stream talk )

# save having to set command line variable
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
