#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single Step Execution", cmd_si},
  { "info","Print Register", cmd_info},
  { "x","Scan Memory",cmd_x},
  { "p","Expression Evaluation",cmd_p},
  { "w","Set New Watchpoint",cmd_w},
  { "d","Delete Watchpoint",cmd_d},
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
    int i=0;
    char *arg=strtok(NULL, " ");
    if(!arg)
	cpu_exec(1);
    else
    {
	sscanf(arg,"%d",&i);
	if(i<=0)
	    cpu_exec(-1);
	else
	    cpu_exec(i);
    }
    return 0;
}

static int cmd_info(char *args){
    char *arg=strtok(NULL," ");
    int i;
    if(strcmp(arg,"r")==0)
    {
        for(i=0;i<8;i++)
	{    printf("%s:\t0x%08x\t%d",regsl[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
	     printf("\n");
	}
	for(i=0;i<8;i++)
	{    printf("%s:\t0x%08x\t%d",regsw[i],cpu.gpr[i]._16,cpu.gpr[i]._16);
	     printf("\n");
	}
	for(i=0;i<8;i++)
	{
	    for(int j=0;j<2;j++)
	    {    printf("%s:.\t0x%08x\t%d",regsb[i],cpu.gpr[i]._8[j],cpu.gpr[i]._8[j]);
		 printf("\n");
	    }
	}
    }
    else if(strcmp(arg,"w")==0)
    {
	list_watchpoint();
    }
    return 0;
}

static int cmd_x(char *args){
    vaddr_t ad;
    int num;
    char *arg_n=strtok(NULL," ");
    sscanf(arg_n,"%d",&num);
    char *arg_s=strtok(NULL," ");
    sscanf(arg_s,"%x",&ad);
    printf("Address		Dword block		Byte sequence");
    printf("\n");
    int i,j;
    for(i=0;i<num;i++)
    {    printf("0x%08x\t",ad);
	 printf("0x%08x\t",vaddr_read(ad,4));
	 for(j=1;j<=4;j++)
	 {
	    printf("%02x ",vaddr_read(ad,j)>>(8*(j-1)));
	 }
	 printf("\n");
	 ad+=4;
    }
    return 0;
}

static int cmd_p(char *args){
    char *arg=strtok(NULL,"&");
    //printf("%s\n",arg);
    bool success=true;
    int result=expr(arg,&success);
    printf("Result: %d\n",result);
    return 0;
}

static int cmd_w(char *args){
    set_watchpoint(args);
    return 0;
}

static int cmd_d(char *args){
    char *arg=strtok(NULL," ");
    int NO;
    sscanf(arg,"%d",&NO);
    delete_watchpoint(NO);
    return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
