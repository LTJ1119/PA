#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ=0, TK_TEN=10, TK_SIXTEEN=16, TK_REGS=255

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-",'-'},		//sub
  {"\\*",'*'},		//mul
  {"\\/",'/'},		//div
  {"==", TK_EQ},         // equal
  {"[0-9]+",TK_TEN},	//10
  {"0x[0-9a-f]+",TK_SIXTEEN},	//16
  {"\\$[a-ehilpx]{2,3}",TK_REGS},	//reg
  {"\\(",'('},
  {"\\)",')'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
	  case TK_NOTYPE:break;
	  case '+':
	  case'-':
	  case'*':
	  case'/':
	  case'(':
	  case')':
	  case TK_EQ:
	  case TK_TEN:
	  case TK_SIXTEEN:
	  case TK_REGS:
	  {    tokens[nr_token].type=rules[i].token_type;
	       strncpy(tokens[nr_token].str,substr_start,substr_len);
	       nr_token++;
	  }break;
          default: assert(0);
        }

        break;
      }
    }
    
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int start,int end){
    if((tokens[start].type!='(')||(tokens[end-1].type!=')'))
	return false;
    int judge=0;
    for(int i=start+1;i<end-1;i++)
    {
	if(tokens[i].type=='(')
	    judge++;
	else if(tokens[i].type==')')
	    judge--;
	else
	    continue;
	if(judge<0)
	    return false;
    }
    if(judge!=0)
	return false;
    return true;  
}

bool check_token(int i){
    switch(tokens[i].type){
        case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	{return true;}break;
     }
    return false;
}

bool compare_priorities(int i,int j){
    int x=0;
    int y=0;
    if(tokens[i].type=='*'||tokens[i].type=='/')
	x=1;
    if(tokens[j].type=='*'||tokens[j].type=='/')
	y=1;
    else if(x>y||x==y)
	return true;
    return false;
}

uint32_t find_dominated_op(int p,int q){
    int op=p;
    int judge=0;
    for(int i=p+1;i<q;i++)
    {
    	if(check_token(i)==false)
	    continue;
	if(tokens[i].type=='(')
	{
	    judge=1;
	    for(i++;judge!=0;i++)
	    {
		if(tokens[i].type==')')
		    judge--;
		if(tokens[i].type=='(')
		    judge++;
		else
		    continue;
	    }
	}
	else if(tokens[op].type=='(')
	{
	    op=i;
	    continue;
	}
	else if(compare_priorities(op,i)==true)
	    op=i;
    }
    return op;
}

uint32_t expr(char *e, bool *success) {
  
   bool a=make_token(e);
   if(check_parentheses(0,nr_token)==true)
	    printf("true expression!\n");
    else
	    printf("false expression!\n");
    int op=find_dominated_op(0,nr_token);
    printf("%s\n",tokens[op].str);
   if (!a) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
