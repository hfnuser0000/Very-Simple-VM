#include <iostream>
#include <vector>
#include <map>
using namespace std;

//#define debug

//Resource type
enum {
	EAX = 0x00, //General purpose register EAX
	EBX,		//General purpose register EBX
	CMX,		//Store comparision result
	MEM = 0x10, //Memory
	STA = 0x20, //Top of stack
	PTR = 0x30, //Pointer
	CON = 0x40	//Constant
};

//Instruction set
enum {
	//Load and store value
	MOV, //source, destination
	LBL, //label_id: create a label, label_id should be unique

	//Alrithmetic operators
	ADD, //a, b, c: set c=a+b
	SUB, //a, b, c: set c=a-b
	MUL, //a, b, c: set c=a*b
	DIV, //a, b, c: set c=a/b
	CMP, //a, b set a-b to cmx

	//Bitwise operators
	BIA, //a, b, c: c=a&b
	BIO, //a, b, c: c=a|b
	BIX, //a, b, c: c=a^b;

	NOT, //a, b: b=!a;

	//Branching
	JMP, //label_id: jump to label
	JEQ, //label_id: jump if cmx == 0
	JNE, //label_id: jump if cmx != 0
	JLT, //label_id: jump if cmx <  0
	JGT, //label_id: jump if cmx >  0
	JLE, //label_id: jump if cmx <= 0
	JGE, //label_id: jump if cmx >= 0
	CAL, //label_id: jump to label, add pc to proc_stack
	CEQ, //label_id: jump if cmx == 0, add pc to proc_stack
	CNE, //label_id: jump if cmx != 0, add pc to proc_stack
	CLT, //label_id: jump if cmx <  0, add pc to proc_stack
	CGT, //label_id: jump if cmx >  0, add pc to proc_stack
	CLE, //label_id: jump if cmx <= 0, add pc to proc_stack
	CGE, //label_id: jump if cmx >= 0, add pc to proc_stack
	NAV, //native_fn_id: call a native function
	RET, // : restore pc from proc_stack
	EXI,  // : exit program

	//Unofficial instructions(for debugging)
	PRINT,
	INPUT,
};

class VirtualMachine {
private:
	int pc;
	vector<int> code;
	vector<int> memory;
	vector<int> stack;
	vector<int> proc_stack;
	map<int, int> label;

	//flags
	bool logic_error_flag, stop_flag;

	//Registers
	int eax,
		ebx,
		cmx; //for comparision

	int nextCode() {
		if (pc >= code.size()) {
			stop_flag = true;
			return EXI;
		}
		return code[pc++];
	}

	void push(int v) {
		stack.push_back(v);
	}

	int top() {
		if (stack.empty()) {
			logic_error_flag = true;
			return 0;
		}
		return stack.back();
	}

	int pop() {
		if (stack.empty()) {
			logic_error_flag = true;
			return 0;
		}
		int top = stack.back();
		stack.pop_back();
		return top;
	}

	void proc_push(int v) {
		proc_stack.push_back(v);
	}

	int proc_restore() {
		if (proc_stack.empty()) {
			logic_error_flag = true;
			return 0;
		}
		int top = proc_stack.back();
		proc_stack.pop_back();
		return top;
	}

	int mem(int addr) {
		if (addr < 0 || addr >= memory.size()) {
			logic_error_flag = true;
			return 0;
		}
		return memory[addr];
	}

	void set_mem(int addr, int v) {
		if (addr < 0 || addr >= memory.size()) {
			logic_error_flag = true;
			return;
		}
		memory[addr] = v;
	}

	int read() {
		static int type;
		type = nextCode();
		switch (type) {
		case EAX:
#ifdef debug
			cout << "EAX ";
#endif
			return eax;
			break;
		case EBX:
#ifdef debug
			cout << "EBX ";
#endif
			return ebx;
			break;
		case CMX:
#ifdef debug
			cout << "CMX ";
#endif
			return cmx;
			break;
		case MEM:
#ifdef debug
			cout << "MEM ";
#endif
			return mem(nextCode());
			break;
		case STA:
#ifdef debug
			cout << "STA ";
#endif
			return pop();
			break;
		case PTR:
#ifdef debug
			cout << "PTR ";
#endif
			return mem(read());
			break;
		case CON:
#ifdef debug
			cout << "CON ";
#endif
			return nextCode();
			break;
		default:
#ifdef debug
			cout << "UNKNOWN ";
#endif
			break;
		}
	}

	void write(int v) {
		static int type;
		type = nextCode();
		switch (type) {
		case EAX:
#ifdef debug
			cout << "EAX ";
#endif
			eax = v;
			break;
		case EBX:
#ifdef debug
			cout << "EBX ";
#endif
			ebx = v;
			break;
		case CMX:
#ifdef debug
			cout << "CMX ";
#endif
			cmx = v;
			break;
		case MEM:
#ifdef debug
			cout << "MEM ";
#endif
			set_mem(nextCode(), v);
			break;
		case STA:
#ifdef debug
			cout << "STA ";
#endif
			push(v);
			break;
		case PTR:
#ifdef debug
			cout << "PTR ";
#endif
			set_mem(read(), v);
			break;
		default:
#ifdef debug
			cout << "UNKNOWN ";
#endif
			break;
		}
	}

	int lbl(int id) {
		if (!label[id]) {
			logic_error_flag = true;
			return 0;
		}
		return label[id];
	}

public:
	VirtualMachine(int memory_size)
		: memory(memory_size, 0)
	{
		proc_stack.reserve(1024);
		pc = 0;
		logic_error_flag = false;
		stop_flag = false;
		eax = 0;
		ebx = 0;
		cmx = 0;
	}

	void fetch(vector<int> p_code) {
		code = p_code;

		//label resolve
		for (int i = 0; i < code.size(); ++i) {
			if (code[i] == LBL) {
				if (i + 1 < code.size())
					label[code[i + 1]] = i + 2;
				else
					logic_error_flag = true;
			}
		}
	}

	void run() {
		while (!stop_flag) {
			if (logic_error_flag) {
				cout << "Oops! This code has some errors." << endl;
				break;
			}
			int opcode = nextCode();
			int a, b;
			switch (opcode) {
			case MOV:
#ifdef debug
				cout << "MOV ";
#endif
				write(read());
				break;
			case LBL:
#ifdef debug
				cout << "LBL ";
#endif
				nextCode();
				break;
			case ADD:
#ifdef debug
				cout << "ADD ";
#endif
				write(read() + read());
				break;
			case SUB:
#ifdef debug
				cout << "SUB ";
#endif
				write(read() - read());
				break;
			case MUL:
#ifdef debug
				cout << "MUL ";
#endif
				write(read() * read());
				break;
			case DIV:
#ifdef debug
				cout << "DIV ";
#endif
				write(read() / read());
				break;
			case CMP:
#ifdef debug
				cout << "CMP ";
#endif
				cmx = read() - read();
				break;
			case BIA:
#ifdef debug
				cout << "BIA ";
#endif
				write(read() & read());
				break;
			case BIO:
#ifdef debug
				cout << "BIO ";
#endif
				write(read() | read());
				break;
			case BIX:
#ifdef debug
				cout << "BIX ";
#endif
				write(read() ^ read());
				break;
			case NOT:
#ifdef debug
				cout << "NOT ";
#endif
				write(!read());
				break;
			case JMP:
#ifdef debug
				cout << "JMP ";
#endif
				pc = lbl(nextCode());
				break;
			case JEQ:
#ifdef debug
				cout << "JEQ ";
#endif
				a = nextCode();
				if (cmx == 0)
					pc = lbl(a);
				break;
			case JNE:
#ifdef debug
				cout << "JNE ";
#endif
				a = nextCode();
				if (cmx != 0)
					pc = lbl(a);
				break;
			case JLT:
#ifdef debug
				cout << "JLT ";
#endif
				a = nextCode();
				if (cmx < 0)
					pc = lbl(a);
				break;
			case JGT:
#ifdef debug
				cout << "JGT ";
#endif
				a = nextCode();
				if (cmx > 0)
					pc = lbl(a);
				break;
			case JLE:
#ifdef debug
				cout << "JLE ";
#endif
				a = nextCode();
				if (cmx <= 0)
					pc = lbl(a);
				break;
			case JGE:
#ifdef debug
				cout << "JGE ";
#endif
				a = nextCode();
				if (cmx >= 0)
					pc = lbl(a);
				break;
			case CAL:
#ifdef debug
				cout << "CAL ";
#endif
				proc_push(pc + 1);
				pc = lbl(nextCode());
				break;
			case CEQ:
#ifdef debug
				cout << "CEQ ";
#endif
				a = nextCode();
				if (cmx == 0) {
					proc_push(pc + 1);
					pc = lbl(a);
				}
				break;
			case CNE:
#ifdef debug
				cout << "CNE ";
#endif
				a = nextCode();
				if (cmx != 0) {
					proc_push(pc + 1);
					pc = lbl(a);
				}
				break;
			case CLT:
#ifdef debug
				cout << "CLT ";
#endif
				a = nextCode();
				if (cmx < 0) {
					proc_push(pc + 1);
					pc = lbl(a);
				}
				break;
			case CGT:
#ifdef debug
				cout << "CGT ";
#endif
				a = nextCode();
				if (cmx > 0) {
					proc_push(pc + 1);
					pc = lbl(a);
				}
				break;
			case CLE:
#ifdef debug
				cout << "CLE ";
#endif
				a = nextCode();
				if (cmx <= 0) {
					proc_push(pc + 1);
					pc = lbl(a);
				}
				break;
			case CGE:
#ifdef debug
				cout << "CGE ";
#endif
				a = nextCode();
				if (cmx >= 0) {
					proc_push(pc + 1);
					pc = lbl(a);
				}
				break;
			case NAV:
#ifdef debug
				cout << "NAV ";
#endif
				break;
			case RET:
#ifdef debug
				cout << "RET ";
#endif
				pc = proc_restore();
				break;
			case EXI:
#ifdef debug
				cout << "EXI ";
#endif
				stop_flag = true;
				break;
			case PRINT:
#ifdef debug
				cout << "PRINT ";
#endif
				cout << read() << endl;
				break;
			case INPUT:
#ifdef debug
				cout << "INPUT ";
#endif
				cin >> a;
				write(a);
				break;
			default:
#ifdef debug
				cout << "UNKNOWN ";
#endif
				break;
			}
		}
	}
};

int main() {
	VirtualMachine fibonacci(1024);
	fibonacci.fetch(vector<int> {
		//goto main
		JMP, 0,

		//.lbl_2:
		//return 0;
		LBL, 2,
		MOV, CON, 0, STA,
		RET,

		//.lbl_4:
		//stack.push(b)
		//return
		LBL, 4,
		MOV, MEM, 2, STA,
		RET,

		//.loop:
		//if not(i<n) goto lbl_4
		//c = b
		//b = b + a
		//a = c
		//i = i + 1
		//goto loop
		LBL, 3,
		CMP, MEM, 3, MEM, 0,
		JGE, 4,
		MOV, MEM, 2, MEM, 4,
		ADD, MEM, 2, MEM, 1, MEM, 2,
		MOV, MEM, 4, MEM, 1,
		ADD, MEM, 3, CON, 1, MEM, 3,
		JMP, 3,

		//.fibonacci:
		//n = stack.pop()
		//a = 0
		//b = 1
		//if(n < 1) goto lbl_2
		//i = 1
		//goto loop
		LBL, 1,
		MOV, STA, MEM, 0,
		MOV, CON, 0, MEM, 1,
		MOV, CON, 1, MEM, 2,
		CMP, MEM, 0, CON, 1,
		JLT, 2,
		MOV, CON, 1, MEM, 3,
		JMP, 3,

		//.main:
		LBL, 0,
		INPUT, EAX,
		MOV, EAX, STA,
		CMP, EAX, CON, 0,
		JLE, 5,
		CAL, 1,
		PRINT, STA,
		JMP, 0,
		LBL, 5,
		EXI
	});
	fibonacci.run();

	VirtualMachine counting(1024);
	counting.fetch(vector<int> {
		JMP, 0,
		LBL, 1,
		ADD, MEM, 0, CON, 1, MEM, 0,
		JMP, 2,
		LBL, 0,
		MOV, CON, 0, MEM, 0,
		LBL, 2,
		CMP, MEM, 0, CON, 100000,
		JLT, 1,
		PRINT, MEM, 0,
		EXI
	});
	//counting.run();
	system("pause");
	return 0;
}