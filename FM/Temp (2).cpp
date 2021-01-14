#include <string>
#include <iostream>
#include <list>
#include <vector>
#include "os_file.h"

using namespace std;

class Path
{
private:
	const char* path;
	void destroyPathOnFiles();
public:

	vector<string> filesOfPath;
	Path(const char* path);
	~Path();

	const char* getPath();
	void setPath(const char* path);
	int CorrectlyFilesName();
	char getPathByIndex(int index);
};

class File
{
private:
	string name;
	int size;
public:
	File();
	File(string name, int size);
	File(const File* file);
	~File() = default;
	string getName();
	void setName(string name);
	int getSize();
	void setSize(int size);

};


class Directory : public File
{
private:
	Directory* parentDir = nullptr;
public:
	list<File*> files; //files in
	Directory();
	~Directory() = default;

	Directory* getParentDir();
	void setParentDir(Directory* NewParentDir);

	int cleanDir();
};

class FileManager
{
private:
	int diskSize = 0;
	bool created = false;
	Directory* rootDir = nullptr;
	Directory* nowDir = nullptr;
	Directory* workDir = nullptr;
	int change_work_dir(Path& filePath);
	int canAddFile(int fileSize); // file size>disk size (?) x
	int searchFile(string fileName);
public:
	FileManager();
	~FileManager();
	int create(int diskSize);
	int destroy();
	int create_dir(const char* path);
	int create_file(const char* path, int fileSize);
	int remove(const char* path, int rec);
	int change_dir(const char* path);
	void getCurDir(char* dst);
	int move(const char* oldPath, const char* newPath);
};


Path::Path(const char* path)
{
	setPath(path);
	destroyPathOnFiles();
}

Path::~Path() = default;;

const char* Path::getPath()
{
	return path;
}

void Path::setPath(const char* path)
{
	this->path = path;
}

void Path::destroyPathOnFiles()
{
	string fileName = "";
	for (int i = 0; path[i] != '\0'; i++)
	{
		if (path[i] == '/')
		{
			if (!fileName.empty())
			{
				filesOfPath.push_back(fileName);
				fileName.clear();
			}
		}
		else
		{
			fileName += path[i];
		}
	}
	if (!fileName.empty())
	{
		filesOfPath.push_back(fileName);
		fileName.clear();
	}
}

char Path::getPathByIndex(int index)
{
	return this->path[index];
}

int Path::CorrectlyFilesName()
{
	for (string& name : filesOfPath)
	{
		if (name.size() > 32)
		{
			return 0;
		}
		for (char& element : name)
		{
			if (!((element >= 65) && (element <= 90)))
			{
				if (!((element >= 97) && (element <= 122)))
				{
					if (!((element >= 48) && (element <= 57)))
					{
						if (!((element == '.' || (element == '_'))))
						{
							return 0;
						}
					}
				}
			}
		}
		return 1;
	}
}

File::File() {}

File::File(string name, int size)
{
	setName(name);
	setSize(size);
}

File::File(const File * file)
{
	setName(file->name); //меняем значение 
	setSize(file->size);
}

string File::getName()
{
	return name;
}

void File::setName(string name)
{

	this->name = name;
}

int File::getSize()
{
	return size;
}

void File::setSize(int size)
{
	this->size = size;
}

Directory::Directory()
{
	setSize(0);
}

int Directory::cleanDir()
{
	if (files.empty())
		return 0;
	int dimension = 0;
	list<File*>::iterator file = files.begin(); //iterator-file, shows the 1 element
	while (file != files.end())
	{
		if ((*file)->getSize() == 0)
		{
			((Directory*)(*file))->cleanDir();

		}
		dimension += (*file)->getSize();
		delete* file;
		files.erase(file++);
	}
	return dimension;
}



Directory* Directory::getParentDir()
{
	return parentDir;
}

void Directory::setParentDir(Directory * NewParentDir)
{
	this->parentDir = NewParentDir;
}

FileManager::FileManager() {}
FileManager::~FileManager() {}

int FileManager::create(int diskSize)
{
	if (diskSize <= 0)
	{
		return 0;
	}

	if (created == true)
	{
		return 0;
	}

	this->diskSize = diskSize;
	rootDir = new Directory();
	rootDir->setName("");
	rootDir->setParentDir(nullptr);
	nowDir = rootDir;
	workDir = rootDir;
	created = true;

	return 1;
}


int FileManager::destroy()
{
	if (created == false)
	{
		return 0;
	}

	rootDir->cleanDir();
	delete rootDir;
	diskSize = 0;
	nowDir = nullptr;
	workDir = nullptr;
	rootDir = nullptr;
	created = false;

	return 0;
}

int FileManager::create_dir(const char* path)
{
	if (created == false) return 0;
	Path filePath(path);
	if (filePath.filesOfPath.empty()) return 0;
	if (!filePath.CorrectlyFilesName()) return 0;

	Directory* newDir = new Directory();
	string newDirName = filePath.filesOfPath.back();
	if ((newDirName == ".") || (newDirName == ".."))
	{
		return 0;
	}

	filePath.filesOfPath.pop_back();
	if (!change_work_dir(filePath)) return 0;
	if (searchFile(newDirName)) return 0;
	newDir->setName(newDirName);
	newDir->setParentDir(workDir);
	workDir->files.push_back(newDir);
	return 1;
}

int FileManager::create_file(const char* path, int fileSize)
{
	if (created == false) return 0;
	Path filePath(path);

	if (filePath.filesOfPath.empty()) return 0;
	if (!filePath.CorrectlyFilesName()) return 0;

	File* newFile = new File();
	string newFileName = filePath.filesOfPath.back();

	if ((newFileName == ".") || (newFileName == "..")) return 0;

	filePath.filesOfPath.pop_back();

	if (!change_work_dir(filePath)) return 0;
	if (searchFile(newFileName)) return 0;
	if (!canAddFile(fileSize)) return 0;

	newFile->setName(newFileName);
	newFile->setSize(fileSize);
	workDir->files.push_back(newFile);
	return 1;
}

int FileManager::remove(const char* path, int rec)
{
	if (created == false) return 0;
	Path file_path(path);
	if (file_path.filesOfPath.empty()) return 0;
	if (!file_path.CorrectlyFilesName()) return 0;

	string fileName = file_path.filesOfPath.back();
	file_path.filesOfPath.pop_back();

	if (!change_work_dir(file_path)) return 0;

	int necessaryFile = 0;
	Directory* tempDir = workDir;

	list<File*>::iterator file = tempDir->files.begin();
	while (file != tempDir->files.end())
	{
		if ((*file)->getName() == fileName)
		{
			if ((*file)->getSize() == 0)
			{
				if ((((Directory*)(*file))->files.empty() != 1) && rec == 0) return 0;
				list<File*>::iterator element = ((Directory*)(*file))->files.begin();
				while (element != ((Directory*)(*file))->files.end())
				{
					string elementsPath(path);
					if (elementsPath.back() == '/')
					{
						elementsPath += (*element)->getName();
					}
					else
					{
						elementsPath += "/" + (*element)->getName();
					}
					if (nowDir->getName() == fileName)
					{
						nowDir = rootDir;
					}
				}
			}
			diskSize += (*file)->getSize();
			delete (*file);
			tempDir->files.erase(file);
			necessaryFile = 1;
			break;
		}
		++file;

	}
	if (necessaryFile == 0)
	{
		return 0;
	}
}

int FileManager::change_dir(const char* path)
{
	if (created == false) return 0;
	Path filePath(path);
	if (!filePath.CorrectlyFilesName()) return 0;
	if (!change_work_dir(filePath)) return 0;
	nowDir = workDir;
	return 0;
}

void FileManager::getCurDir(char* dst)
{
	workDir = nowDir;
	string AllPath = workDir->getName();
	while (workDir->getParentDir() != nullptr)
	{
		workDir = workDir->getParentDir();
		AllPath = workDir->getName() + "/" + AllPath;
	}
	strcpy(dst, AllPath.c_str());
}

int FileManager::move(const char* oldPath, const char* newPath)
{
	if (created == false) return 0;

	Path old_file_path(oldPath);
	Path new_file_path(newPath);

	if (old_file_path.filesOfPath.empty()) return 0;
	if (!old_file_path.CorrectlyFilesName()) return 0;

	if (new_file_path.filesOfPath.empty()) return 0;
	if (!new_file_path.CorrectlyFilesName()) return 0;

	string fileName = old_file_path.filesOfPath.back();
	old_file_path.filesOfPath.pop_back();
	
	if (!change_work_dir(old_file_path)) return 0;
	
	File *buffer = new File();
	for (File*& file : workDir->files)
	{
		if (file->getName() == fileName)
		{
			buffer = file;
		}
	}
	if (!change_work_dir(new_file_path)) return 0;
	workDir->files.push_back(buffer);
	remove(oldPath, 1);
	return 1;
}

int FileManager::change_work_dir(Path & filePath)
{

	if (filePath.getPathByIndex(0) == '/')
		workDir = rootDir;
	else
		workDir = nowDir;
	for (string& file : filePath.filesOfPath)
	{
		if (file == ".")
		{
			workDir = workDir;
		}
		else if (file == "..")
		{
			if (workDir->getParentDir() == nullptr)
			{
				return 0;
			}
			workDir = workDir->getParentDir();
		}
		else
		{
			for (File*& isDirectory : workDir->files)
			{
				if (isDirectory->getSize() == 0)
				{
					if (isDirectory->getName() == file)
						workDir = (Directory*)isDirectory;
				}
			}
			if (workDir->getName() != file)
			{
				return 0;
			}
		}
	}
	return 1;
}

int FileManager::canAddFile(int fileSize)
{
	if (diskSize < fileSize || fileSize <= 0)
	{
		return 0;
	}
	diskSize -= fileSize;
	return 1;
}

int FileManager::searchFile(string fileName)
{
	if (workDir->files.empty())
	{
		return 0;
	}
	for (File*& file : workDir->files)
	{
		if (file->getName() == fileName)
		{
			return 1;
		}
	}
	return 0;
}


FileManager file_manager;
int my_create(int disk_size)
{
	return file_manager.create(disk_size);
}

int my_destroy() {
	return file_manager.destroy();
}
int my_create_dir(const char* path) {
	return file_manager.create_dir(path);
}
int my_create_file(const char* path, int file_size) {
	return file_manager.create_file(path, file_size);
}
int my_remove(const char* path, int recursive) {
	return file_manager.remove(path, recursive);
}
int my_change_dir(const char* path) {
	return file_manager.change_dir(path);
}
void my_get_cur_dir(char* dst) {
	return file_manager.getCurDir(dst);
}
int move(const char* old_path, const char* new_path) {
	return file_manager.move(old_path, new_path);
}

void setup_file_manager(file_manager_t* fm)
{
	fm->create = my_create;
	fm->destroy = my_destroy;
	fm->create_dir = my_create_dir;
	fm->create_file = my_create_file;
	fm->remove = my_remove;
	fm->change_dir = my_change_dir;
	fm->get_cur_dir = my_get_cur_dir;
	fm->move = move;
}
