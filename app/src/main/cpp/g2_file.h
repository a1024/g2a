//g2_file.h - Include for file operations for Grapher 2A.
//Copyright (C) 2020  Ayman Wagih Mohsen
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <https://www.gnu.org/licenses/>.

//
// Created by MSI on 10/5/2020.
//

#ifndef GRAPHER_2_G2_FILE_H
#define GRAPHER_2_G2_FILE_H
extern const char *statedir,//full path to the folder containing the state
				*statefilepath;//full path to file containing the state data
void			sys_check(int line);
#define 		SYS_CHECK()		sys_check(__LINE__)
void			init_directories();
int				saveFile(const char *addr, const char *data, size_t size, bool binary=true);
long			getfilesize(FILE *file);
int				loadFile(const char *addr, char *&data, size_t &size, bool binary=true);
void			directorycontents(const char *dirpath, std::vector<std::string> &ret);
void			mkdir_firsttime(const char *dirpath);
unsigned 		hexstr2uint(const char *text);
int				statefolder_readstate(char *&text, size_t &length);
void			statefolder_deletecontents();
const char*		version2str();
void 			statefolder_writestate(const char *usertext=nullptr);
#endif //GRAPHER_2_G2_FILE_H
