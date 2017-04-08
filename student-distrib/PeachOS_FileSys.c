#include "PeachOS_FileSys.h"
//test
uint32_t file_index;

bootblock_t BootBlock;
dentry_t* dirEntries;
inode_t* inodes;

uint32_t num_directories;
uint32_t num_inodes;
uint32_t num_datablocks;
uint32_t bbAddr;
uint32_t dataStart;


//comment block needed
void fileSystem_init(uint32_t fsAddr)
{
  bbAddr = fsAddr;
  bootblock_t* bb = (bootblock_t*)(bbAddr);

  num_directories = bb->num_directories;
  num_inodes = bb->num_inodes;
  num_datablocks = bb->num_datablocks;

  dirEntries = (dentry_t*)(bb->dirEntries);
  inodes = (inode_t*)(bbAddr + BLOCK_SIZE);
  dataStart = (uint32_t)(inodes + num_inodes);
}

/*
 * read_dentry_by_name
 *  DESCRIPTION:
 *          This function uses the filename to access a file and store its
 *					directory entry info into the struct pointed by dentry
 *  INPUT: fname - the name of the file
 *				 dentry - a pointer to the directory entry struct to be filled
 *  OUTPUT: none
 *  RETURN VALUE: return 0 on sucess, -1 otherwise
*/

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
  int i;
  int len = strlen((int8_t*)(fname));
  if (len > FILENAMESIZE)
	{
		return -1;
	}

  for (i = 0; i < DIRENTRIES; i++)
  {
    if ((strncmp(dirEntries[i].filename,  (int8_t*)(fname), len)) == 0)
    {
       strncpy(dentry->filename, dirEntries[i].filename, len);
       dentry->filetype = dirEntries[i]. filetype;
       dentry->inode = dirEntries[i].inode;
       return 0;
    }
  }
  return -1;
}

/*
 * read_dentry_by_index
 *  DESCRIPTION:
 *          This function uses an index access a file and store its
 *					directory entry info into the struct pointed by dentry
 *  INPUT: index - index of the block corresponding to the file
 *				 dentry - a pointer to the directory entry struct to be filled
 *  OUTPUT: none
 *  RETURN VALUE: return 0 on sucess, -1 otherwise
*/

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
  if (index >= DIRENTRIES || index < 0)
  {
    return -1;
  }
  strncpy(dentry->filename, dirEntries[index].filename, FILENAMESIZE);
  dentry->filetype = dirEntries[index].filetype;
  dentry->inode = dirEntries[index].inode;

  return 0;
}

/*
 * read_data
 *  DESCRIPTION:
 *          This function reads "length" bytes from the starting position
 *				  "offset" in the file with inode and returns the number of bytes
 *				  read and placed in buf
 *  INPUT: inode - index number to identify the file
 *				 offset - the starting position offset to access a file
 *				 buf - the buffer to write into from the file
 *				 length - the number of bytes to be placed in the buffer
 *  OUTPUT: none
 *  RETURN VALUE: returns the number of bytes read and placed in buf, -1 otherwise
*/

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	int i, k;
	uint32_t file_length, db_start_idx, db_start_byte_idx,
	num_bytes_to_copy, num_db_needed, byte_index;
	char * byte_array;
	inode_t * db_array;
  	inode_t * dest_inode;
	uint32_t * inode_data_array;

	//Checking if inode is invalid (if the index is greater than N - 1)
	if (inode >= num_inodes)
		return -1;
	dest_inode = inodes + inode; //points to the inode
	//make an array of data block indices
	inode_data_array = (uint32_t *)dest_inode;
	file_length = inode_data_array[0];//first index
	inode_data_array += 1;

	if(file_length == 0 || length == 0) return 0;

	//Check if start address is greater than file length
	if (offset >= file_length)
	{
			return -1;
	}

	//make a data block array and get first data block, and first byte index
	db_array = (inode_t *) bbAddr + num_inodes + 1;
	db_start_idx = offset / (BLOCK_SIZE); // CHANGE MADE
	db_start_byte_idx = offset % (BLOCK_SIZE); // CHANGE MADE

	num_bytes_to_copy = (file_length - 1 <= length + offset - 1)? file_length - offset : length;
	num_db_needed = ((num_bytes_to_copy + db_start_byte_idx - 1)/ BLOCK_SIZE) + 1;

	k = 0; i = db_start_idx; byte_index = db_start_byte_idx; // 5
  for ( ; i < db_start_idx + num_db_needed; i++){
		byte_array = (char *)(db_array + inode_data_array[i]); // get the byte array of the db
		while (byte_index < BLOCK_SIZE && k < num_bytes_to_copy){
			buf[k++] = byte_array[byte_index++];
		}
    byte_index = 0;

	}
  return num_bytes_to_copy; 
}

/*
 * print_directory
 *  DESCRIPTION:
 *          This function reads "length" bytes from the starting position
 *				  "offset" in the file with inode and returns the number of bytes
 *				  read and placed in buf
 *  INPUT: inode - index number to identify the file
 *				 offset - the starting position offset to access a file
 *				 buf - the buffer to write into from the file
 *				 length - the number of bytes to be placed in the buffer
 *  OUTPUT: none
 *  RETURN VALUE: returns the number of bytes read and placed in buf, -1 otherwise
*/
int32_t print_directory()
{
	dentry_t toprint;

	uint8_t file_name_string[FILENAMESIZE+1];
	uint8_t byte_string[MAX_BYTE_WIDTH];
	uint8_t formatted_byte_string[MAX_BYTE_WIDTH+1];
	uint8_t file_type_string[sizeof(uint32_t)+1];

	int i, j, file_len, byte_len;

	for(i=0; i<num_directories; i++)
	{
		if(read_dentry_by_index(i, &toprint)==0)
		{
			//get the number of bytes in each file and the file name length
			file_len = strlen((char*)(toprint.filename));
			if(file_len > 32) file_len = 32;

			itoa(*(uint32_t*)(inodes+toprint.inode), (char*)byte_string, 10);
			itoa(toprint.filetype, (char*)file_type_string, 10);
			byte_len = strlen((char*)(byte_string));

			//format string for printing by filling with spaces
			for(j=0; j<FILENAMESIZE; j++)
			{
				file_name_string[j] = ' ';
			}

			//right justify string for printing
			for(j=0; j<file_len; j++)
			{
				file_name_string[j-file_len+FILENAMESIZE] = toprint.filename[j];
			}
			file_name_string[FILENAMESIZE] = 0x0;

			//format string for printing by filling with spaces
			for(j=0; j<MAX_BYTE_WIDTH; j++)
			{
				formatted_byte_string[j] = ' ';
			}

			//right justify string for printing
			for(j=0; j<byte_len; j++)
			{
				formatted_byte_string[j-byte_len+MAX_BYTE_WIDTH] = byte_string[j];
			}
			formatted_byte_string[MAX_BYTE_WIDTH] = 0x0;

            terminal_write(1, (char*)"file_name: ", 0);
            terminal_write(1, (char*)file_name_string, 0);
            terminal_write(1, (char*)", file_type: ", 0);
            terminal_write(1, (char*)file_type_string, 0);
            terminal_write(1, (char*)", file_size: ", 0);
            terminal_write(1, (char*)formatted_byte_string, MAX_BYTE_WIDTH);
			terminal_write(1, (char*)"\n", 1);
		}
	}
	return 0;
}

/*
 * print_file_by_name
 *  DESCRIPTION:
 *          This function reads "length" bytes from the starting position
 *				  "offset" in the file with inode and returns the number of bytes
 *				  read and placed in buf
 *  INPUT: inode - index number to identify the file
 *				 offset - the starting position offset to access a file
 *				 buf - the buffer to write into from the file
 *				 length - the number of bytes to be placed in the buffer
 *  OUTPUT: none
 *  RETURN VALUE: returns the number of bytes read and placed in buf, -1 otherwise
*/
int32_t print_file_by_name(const uint8_t* fname)
{
	dentry_t toprint;

	uint32_t num_bytes_read;
	uint32_t offset = 0;
	uint32_t length = 37600; //larger than largest file in system. Can be changed as needed
	uint8_t buf[38000] = {0}; 
	uint8_t file_name[FILENAMESIZE+1] = {0};
	uint8_t printbuf[2] = {0};
	int i;

	//strncpy((char*)file_name, (char*)(fname), FILENAMESIZE);

	if(read_dentry_by_name(fname, &toprint)==0)
	{
		num_bytes_read = read_data(toprint.inode, offset, buf, length);

		//print buffer to terminal one char at a time (doesn't print 0's)
		for(i=0; i<num_bytes_read; i++)
		{
			printbuf[0] = buf[i];
			if(printbuf[0] == NULL) printbuf[0] = ' ';
		    terminal_write(1, (char*)printbuf, 0);
		}

		//copy filename (32 bytes) to file_name (33 bytes, null terminated) for printing
		strncpy((char*)file_name, (char*)(toprint.filename), FILENAMESIZE);

		terminal_write(1, (char*)"\n", 0);
		terminal_write(1, (char*)"file_name: ", 0);
		terminal_write(1, (char*)file_name, 0);
	}
	else
	{
		terminal_write(1, (char*)"File does not exist. \n", 0);
		return -1;
	}

	return 0;
}

/*
 * print_file_by_index
 *  DESCRIPTION:
 *          This function reads "length" bytes from the starting position
 *				  "offset" in the file with inode and returns the number of bytes
 *				  read and placed in buf
 *  INPUT: inode - index number to identify the file
 *				 offset - the starting position offset to access a file
 *				 buf - the buffer to write into from the file
 *				 length - the number of bytes to be placed in the buffer
 *  OUTPUT: none
 *  RETURN VALUE: returns the number of bytes read and placed in buf, -1 otherwise
*/

int32_t print_file_by_index(uint32_t file_num)
{
	dentry_t toprint;

	uint32_t num_bytes_read;
	uint32_t offset = 0;
	uint32_t length = 37600; //larger than largest file in system. Can be changed as needed
	uint8_t buf[38000] = {0}; 
	uint8_t file_name[FILENAMESIZE+1] = {0};
	uint8_t printbuf[2] = {0};
	int i;

	if(read_dentry_by_index(file_num, &toprint)==0)
	{
		num_bytes_read = read_data(toprint.inode, offset, buf, length);

		//print buffer to terminal one char at a time (doesn't print 0's)
		for(i=0; i<num_bytes_read; i++)
		{
			printbuf[0] = buf[i];
			if(printbuf[0] == NULL) printbuf[0] = ' ';
		    terminal_write(1, (char*)printbuf, 0);
		}

		//copy filename (32 bytes) to file_name (33 bytes, null terminated) for printing
		strncpy((char*)file_name, (char*)(toprint.filename), FILENAMESIZE);

		terminal_write(1, (char*)"\n", 0);
		terminal_write(1, (char*)"file_name: ", 0);
		terminal_write(1, (char*)file_name, 0);
	}
	else
	{
		terminal_write(1, (char*)"File does not exist.\n", 0);
		return -1;
	}
	return 0;
}

/*
 * open_file
 *  DESCRIPTION:
 *          This function accesses and returns the file descriptor for a
 *					file identified by a given filename
 *  INPUT: filename - the string for the filename
 *  OUTPUT: none
 *  RETURN VALUE: the index of the open file in the file array (the file
 *								descriptor), return -1 if unable to open file
*/
int32_t open_file(const uint8_t * filename){
	return -1;
}
/*
 * close_file
 *  DESCRIPTION:
 *          This function closes a file identified by the file descriptor
 *  INPUT: fd = the file descriptor(index of the file in the file array)
 *  OUTPUT: none
 *  RETURN VALUE: return 0 if successful, -1 otherwise
*/
int32_t close_file(int32_t fd){
	return -1;
}
/*
 * read_file
 *  DESCRIPTION:
 *          This function uses the file descriptor (the index of the file
 * 					array) to read a file and copy it to the buf array
 *  INPUT: fd = the file descriptor(index of the file in the file array)
 *				 buf - the array/buffer to write the file info into
 *  			 nbytes - the number of bytes to be copies into buf
 *  OUTPUT: none
 *  RETURN VALUE: number from bytes copied to buf, -1 if unsuccessful
*/
int32_t read_file(int32_t fd, void * buf, int32_t nbytes){
	return -1;
}
/*
 * write_file
 *  DESCRIPTION:
 *          This function uses the file descriptor (the index of the file
 * 					array) to access a file and write to it from buf
 *  INPUT: fd = the file descriptor(index of the file in the file array)
 *				 buf - the array/buffer to read from and write into the file
 *  			 nbytes - the number of bytes to be written to the file
 *  OUTPUT: none
 *  RETURN VALUE: number from bytes to be written to file, -1 if unsuccessful
*/
int32_t write_file(int32_t fd, const void * buf, int32_t nbytes){
	return -1;
}

/*
 * open_directory
 *  DESCRIPTION:
 *          This function accesses and returns the file descriptor for a
 *					directory identified by a given directory name
 *  INPUT: dname - the string for the directory name
 *  OUTPUT: none
 *  RETURN VALUE: the index of the open directory in the file array (the
 *								file descriptor), return -1 if unable to open file
*/
int32_t open_directory(const uint8_t * dname){
	return -1;
}
/*
 * close_directory
 *  DESCRIPTION:
 *          This function closes a directory identified by the file descriptor
 *  INPUT: fd = the file descriptor
 *  OUTPUT: none
 *  RETURN VALUE: return 0 if successful, -1 otherwise
*/
int32_t close_directory(int32_t fd){
	return -1;
}

/*
 * read_directory
 *  DESCRIPTION:
 *          This function uses the file descriptor to read a directory
 * 					and copy it to the buf array
 *  INPUT: fd = the file descriptor
 *				 buf - the array/buffer to write the directory info into
 *  			 nbytes - the number of bytes to be copied into buf
 *  OUTPUT: none
 *  RETURN VALUE: number from bytes copied to buf, -1 if unsuccessful
*/
int32_t read_directory(int32_t fd, void * buf, int32_t nbytes){
	return -1;
}
/*
 * write_directory
 *  DESCRIPTION:
 *          This function uses the file descriptorto access a directory
 * 				  and write to it from buf
 *  INPUT: fd = the file descriptor
 *				 buf - the array/buffer to read from and write into the directory
 *  			 nbytes - the number of bytes to be written to the directory
 *  OUTPUT: none
 *  RETURN VALUE: number from bytes to be written to dir, -1 if unsuccessful
*/
int32_t write_directory(int32_t fd, const void * buf, int32_t nbytes){
	return -1;
}
