#ifndef __COMMANDS_H
  #define __COMMANDS_H

void commands(void);
int logdataCmd(char **argv, unsigned short argc);
int mmc_reset(char **argv, unsigned short argc);
int mmc_multiWTstCmd(char **argv, unsigned short argc);
int mmc_multiRTstCmd(char **argv, unsigned short argc);
int mmc_eraseCmd(char **argv, unsigned short argc);
int mmc_write(char **argv, unsigned short argc);
int mmc_read(char **argv, unsigned short argc);
int mmc_address_zero(char **argv, unsigned short argc); 
int testlaunchCmd(char **argv, unsigned short argc);
int printGyroCmd(char **argv, unsigned short argc);




#endif
    