#include "nemu.h"
#include "cpu/reg.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "monitor/ui.h"
#include "monitor/breakpoint.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
	NOTYPE = 256,
	EQ,
	NEQ,
	LE,
	GE,
	NUM,
	REG,
	SYMB,
	DEREF,
	HEX,
	SL,
	SR,
	AND,
	OR,
	NOT,
	NEG

	/* TODO: Add more token types */

};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE}, // white space
	{"\\+", '+'},
	{"==", EQ},    // equal
	{"[0-9]+", NUM},
	{"0[xX][0-9a-fA-F]+", HEX},
	{"\\$[a-zA-Z]+", REG},
	{"[a-zA-Z_][a-zA-Z0-9_]*", SYMB},
	{"\\*", '*'},
	{"/", '/'},
	{"-", '-'},
	{"%", '%'},
	{"\\^", '^'},
	{"\\|", '|'},
	{"&", '&'},
	{"<", '<'},
	{">", '>'},
	{"<<", SL},
	{">>", SR},
	{"<=", LE},
	{">=", GE},
	{"==", EQ},
	{"\\(", '('},
	{"\\)", ')'},
	{"!=", NEQ},
	{"<=", LE},
	{">=", GE},
	{"<", '<'},
	{">", '>'},
	{"&&", AND},
	{"\\|\\|", OR},
	{"!", NOT},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				// printf("match regex[%d] at position %d with len %d: %.*s", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. 
				 * Add codes to perform some actions with this token.
				 */

				switch (rules[i].token_type) {
					case NOTYPE:
						break;
					case NUM:
					case HEX:
					case REG:
					case SYMB:
						tokens[nr_token].type = rules[i].token_type;
						assert(substr_len < 32);
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = '\0';
						nr_token++;
						break;
					default:
						tokens[nr_token].type = rules[i].token_type;
						nr_token++;
				}
				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

bool check_para(int p,int q,bool *success)
{
	if(tokens[p].type!='(' || tokens[q].type!=')') return false;
	int cnt=0;
	for(int i=p;i<=q;i++)
	{
		if(tokens[i].type=='(') cnt++;
		else if(tokens[i].type==')')
		{
			if(cnt==0) { *success = false; return false; }
			cnt--;
			if(cnt==0 && i<q-1) return false;
		}
	}
	return cnt==0;
}

uint32_t dominant_op(int p,int q)
{
	int op=-1,level=10,cnt=0;
	for(int i=p;i<=q;i++)
	{
		if(tokens[i].type=='(') { cnt++; continue; }
		else if(tokens[i].type==')') { cnt--; continue; }
		if(cnt>0) continue;
		int clevel;
		switch(tokens[i].type) {
			case OR: 
				clevel = 0; break;
			case AND: 
				clevel = 1; break;
			case '|': 
				clevel = 2; break;
			case '^': 
				clevel = 3; break;
			case '&': 
				clevel = 4; break;
			case EQ:
			case NEQ:
				clevel = 5; break;
			case '<':
			case '>':
			case LE:
			case GE:
				clevel = 6; break;
			case SL: 
			case SR: 
				clevel = 7; break;
			case '+': 
			case '-':
			case NEG:
				clevel = 8; break;
			case '%': 
			case '*': 
			case '/': 
				clevel = 9; break;
			case NOT:
			case DEREF:
				clevel = 10; break;
			default: 
				clevel = -1;
		}
		if(clevel>0 && clevel<=level)
			level = clevel, op = i;
	}
	assert(op!=-1);
	return op;
}

uint32_t look_up_symtab(char *sym, bool *success);

uint32_t eval(int p,int q, bool *success)
{
	if(p>q || *success==false) return 0;
	if(p==q)
	{
		assert(tokens[p].type==NUM || tokens[p].type==HEX || tokens[p].type==REG || tokens[p].type==SYMB);
		uint32_t val=0;
		if(tokens[p].type==NUM)
			sscanf(tokens[p].str,"%d",&val);
		else if(tokens[p].type==HEX)
			sscanf(tokens[p].str,"%x",&val);
		else if(tokens[p].type==REG)
		{
			if(strcmp(tokens[p].str,"$eax")==0) val = cpu.eax;
			else if(strcmp(tokens[p].str,"$ecx")==0) val = cpu.ecx;
			else if(strcmp(tokens[p].str,"$edx")==0) val = cpu.edx;
			else if(strcmp(tokens[p].str,"$ebx")==0) val = cpu.ebx;
			else if(strcmp(tokens[p].str,"$esp")==0) val = cpu.esp;
			else if(strcmp(tokens[p].str,"$ebp")==0) val = cpu.ebp;
			else if(strcmp(tokens[p].str,"$esi")==0) val = cpu.esi;
			else if(strcmp(tokens[p].str,"$edi")==0) val = cpu.edi;
			else if(strcmp(tokens[p].str,"$eip")==0) val = cpu.eip;
			else assert(0);
		}
		else if(tokens[p].type==SYMB)	
		{
			val = look_up_symtab(tokens[p].str, success);
			assert(*success);
		}
		return val;
	}
	if(check_para(p,q,success))
		return eval(p+1,q-1,success);
	assert(*success);
	int op=dominant_op(p,q);
	uint32_t val1=eval(p,op-1,success);
	uint32_t val2=eval(op+1,q,success);
	switch(tokens[op].type) {
		case '+': return val1+val2;
		case '-': return val1-val2;
		case '*': return val1*val2;
		case '/': assert(val2!=0); return val1/val2;
		case '%': assert(val2>0); return val1%val2;
		case '<': return val1<val2;
		case '>': return val1>val2;
		case '&': return val1&val2;
		case '^': return val1^val2;
		case '|': return val1|val2;
		case EQ: return val1==val2;
		case LE: return val1<=val2;
		case GE: return val1>=val2;
		case NEQ: return val1!=val2;
		case AND: return val1&&val2;
		case OR: return val1||val2;
		case NOT: return !val2;
		case NEG: return -val2;
		case DEREF: return vaddr_read(val2, SREG_SS, 4);
		default: assert(0);
	}
}

bool check(int i)
{
    return tokens[i].type=='(' || tokens[i].type==EQ || tokens[i].type==NEQ || tokens[i].type==LE || tokens[i].type==GE || 
		tokens[i].type=='*' || tokens[i].type=='/' || tokens[i].type=='%' || tokens[i].type=='<' || tokens[i].type=='>' || 
		tokens[i].type== AND || tokens[i].type== OR || tokens[i].type=='&' || tokens[i].type=='|' || tokens[i].type=='^';
}

uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}
	for(int i=0;i<nr_token;i++)
	{
		if(tokens[i].type=='-' && (i==0 || check(i-1))) tokens[i].type=NEG;
		else if(tokens[i].type=='*' && (i==0 || check(i-1))) tokens[i].type=DEREF;
	}
	return eval(0, nr_token - 1, success);
}