#include "PeachOS_FileSys.h"


int32_t num_directories;
int32_t num_inodes;
int32_t num_datablocks;
dentry_t* dirEntries;
inode_t* inodes;
uint32_t bbAddr;
uint32_t dataStart;

void fileSystem_init(uint32_t fsAddr)
{
  bbAddr = fsAddr;
  bootblock_t* bb = (bootblock_t*)(bbAddr);

  num_directories = bb->num_directories;
  num_inodes = bb->num_inodes;
  num_datablocks = bb->num_datablocks;

  dirEntries = (dentry_t *)(bbAddr + FILESTATS);
  inodes = (inode_t*)(bbAddr + BLOCK_SIZE);
  dataStart = (bbAddr) + (BootBlock.num_inodes+1)*(BLOCK_SIZE);
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

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
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
       strncpy(dentry->filename, dirEntries[i].filename, FILENAMESIZE);
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
  if (index > DIRENTRIES || index < 0)
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
	int i, j, k;
	uint32_t num_inodes, file_length, db_start_idx, db_start_byte_idx,
	num_bytes_to_copy, num_db_needed, byte_index;
	char * byte_array;
	inode * db_array, dest_inode;
	uint32_t * inode_data_array

	// get the number of inodes
  num_inodes = ((bootblock_t *)file_sys_addr)->num_inodes;
	//Checking if inode is invalid (if the index is greater than N - 1)
	if (inode >= num_inodes)
		return -1;

	dest_inode = (inode_t *)(file_sys_addr + BLOCK_SIZE);
	dest_inode = dest_inode[inode]; //points to the inode

	//make an array of data block indices
	inode_data_array = (uint32_t *)dest_inode;
	file_length = inode_data_array[0];//first index
	inode_data_array += 1;

	//Check if start address is greater than file length
	if (offset >= file_length)
		return -1;

	//make a data block array and get first data block, and first byte index
	db_array = (inode_t * t) file_sys_addr + num_inodes + 1;
	db_start_idx = offset / (BLOCK_SIZE - 1);
	db_start_byte_idx = offset % (BLOCK_SIZE - 1);

	num_bytes_to_copy = (file_length - 1 <= length + offset)? file_length - offset : length;
	num_db_needed = (num_bytes_to_copy + db_start_byte_idx + 1)/ BLOCK_SIZE;

	k = 0; i = db_start_idx; byte_index = db_start_byte_idx; // 5
	while (i < db_start_idx + num_db_needed){

		byte_array = (char *)db_array[inode_data_array[i]]; // get the byte array of the db
		while (byte_index < BLOCK_SIZE && k < num_bytes_to_copy]){
			buf[k++] = byte_array[byte_index++];
		}
		i++;

	}












/*
	uint32_t total_num_inodes; 	    //how many inodes? Needed for offset to data blocks
	uint32_t data_block_number;     //0th data block #
	uint32_t data_block_start_address;  //0th data block address
	uint32_t* pointer;			//points to address of first data block for this inode
	uint32_t num_bytes_in_file = *(uint32_t*)(file_sys_addr+ABSOLUTE_BLOCK_SIZE*(inode+1));
	//unsigned char* temp_buffer;
	uint32_t temp_length;
	uint32_t data_block_pointer;
	uint32_t num_blocks;
	unsigned char* temp_pointer;

	unsigned char* largebuffer[40000]={0};
	int i;
	//unsigned char data_block_test_data[length+1];
	//data_block_test_data[length]=0x0;


	total_num_inodes = *(uint32_t*)(file_sys_addr + NUM_DENTRIES_SIZE);
	if(inode >= total_num_inodes || inode < 0) return -1;

	data_block_pointer = (file_sys_addr+(inode+1)*ABSOLUTE_BLOCK_SIZE + LENGTH_IN_BYTES_SIZE);
	data_block_number = *(uint32_t*)(data_block_pointer);
	data_block_start_address = file_sys_addr+(total_num_inodes+data_block_number+1)*ABSOLUTE_BLOCK_SIZE;


	if(offset >= num_bytes_in_file) return 0;

	if(offset+length > num_bytes_in_file)
	{
		length = num_bytes_in_file - offset;
	}

	//largebuffer[40000]=
	num_blocks = (uint32_t)(num_bytes_in_file/ABSOLUTE_BLOCK_SIZE);
	if((num_bytes_in_file%ABSOLUTE_BLOCK_SIZE)!=0) num_blocks++;

	temp_length = num_bytes_in_file;

	for(i=0;i<num_blocks;i++){
		data_block_number = *(uint32_t*)(file_sys_addr+(inode+1)*ABSOLUTE_BLOCK_SIZE + LENGTH_IN_BYTES_SIZE*(i+1));
		pointer = (uint32_t*)(file_sys_addr+(total_num_inodes+data_block_number+1)*ABSOLUTE_BLOCK_SIZE);
		memcpy((char*)largebuffer, (char*)pointer, temp_length);
		temp_length -= ABSOLUTE_BLOCK_SIZE;
	}

	if(length>=num_bytes_in_file) length = num_bytes_in_file;
	if(length+offset>=num_bytes_in_file) length = num_bytes_in_file-offset;
	temp_pointer = (unsigned char*)largebuffer + offset;
	memcpy((char*)buf, (char*)temp_pointer, length);


	return 0;
*/
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
	dentry_t test;
	uint32_t num_files = *(uint32_t*)file_sys_addr;
	uint32_t num_bytes = 0;
	unsigned char file_name_string[FILE_NAME_SIZE+1];
	unsigned char byte_string[MAX_BYTE_WIDTH];
	unsigned char formatted_byte_string[MAX_BYTE_WIDTH+1];
	unsigned char file_type_string[FILE_TYPE_SIZE+1];
	int i, j, file_len, byte_len;

	for(i=0; i<num_files; i++)
	{
		if(read_dentry_by_index(i, &test)==0)
		{
			//get the number of bytes in each file and the file name length
			num_bytes = *(uint32_t*)(file_sys_addr+ABSOLUTE_BLOCK_SIZE*(test.inode_number+1));
			file_len = strlen((char*)(test.file_name));
			itoa(num_bytes, (char*)byte_string, 10);
			itoa(test.file_type, (char*)file_type_string, 10);
			byte_len = strlen((char*)(byte_string));


			//format string for printing by filling with spaces
			for(j=0; j<FILE_NAME_SIZE; j++)
			{
				file_name_string[j] = ' ';
			}

			//right justify string for printing
			for(j=0; j<file_len; j++)
			{
				file_name_string[j-file_len+FILE_NAME_SIZE] = test.file_name[j];
			}
			file_name_string[FILE_NAME_SIZE] = 0x0;

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

            terminal_write(1, (char*)"file_name: ", 11);
            terminal_write(1, (char*)file_name_string, 32);
            terminal_write(1, (char*)", file_type: ", 13);
            terminal_write(1, (char*)file_type_string, 4);
            terminal_write(1, (char*)", file_size: ", 13);
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
	dentry_t test;

	//uint32_t inode = 2;
	uint32_t offset = 0;
	unsigned char buf[37001] = {0};
	uint32_t length = 37000;
		unsigned char toprint[2] = {0};
		int i;

	//copy for printing with terminal_write
	unsigned char file_name[FILE_NAME_SIZE+1];
	strncpy((char*)file_name, (char*)(fname), FILE_NAME_SIZE);
	uint32_t file_length;

	if(read_dentry_by_name(file_name, &test)==0)
	{
		//gets last bit
		file_length = *(uint32_t*)(file_sys_addr+ABSOLUTE_BLOCK_SIZE*(test.inode_number+1));
		offset = 0;
		read_data(test.inode_number, offset, buf, length);

	//	terminal_write(1, (char*)buf, 0);
		for(i=0; i<length; i++)
		{
			toprint[0] = buf[i];
		  terminal_write(1, (char*)toprint, 0);
		}
		terminal_write(1, (char*)"\n", 0);
		terminal_write(1, (char*)"file_name: ", 0);
		terminal_write(1, (char*)file_name, 0);
	}
	else
	{
		terminal_write(1, (char*)"File does not exist.\n", 0);
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
	dentry_t test;

	//uint32_t inode = 2;
	uint32_t offset = 0;
	unsigned char buf[38001] = {0};
	uint32_t length = 38000;
	unsigned char file_name[FILE_NAME_SIZE+1];
		unsigned char toprint[2] = {0};
		int i;

	if(read_dentry_by_index(file_num, &test)==0)
	{
		strncpy((char*)file_name, (char*)(test.file_name), FILE_NAME_SIZE);

		read_data(test.inode_number, offset, buf, length);

		buf[38000] = 0x0;

	//	terminal_write(1, (char*)buf, 0);
		for(i=0; i<length; i++)
		{
			toprint[0] = buf[i];
		  terminal_write(1, (char*)toprint, 0);
		}
		terminal_write(1, (char*)"\n", 0);
		terminal_write(1, (char*)"file_name: ", 0);
		terminal_write(1, (char*)file_name, 0);
	}

	else
	{
		terminal_write(1, (char*)"File does not exist.\n", 0);
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
