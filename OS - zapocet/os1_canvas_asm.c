#include <stdio.h>
#include <stdlib.h>

char *empty = ".";
char *vsegment = "|";
char *hsegment = "-";
char *vhsegment = "+";
char *newline = "\n";

struct canvas {
	unsigned int width;
	unsigned int height;
	char *content;
};

__declspec(naked) void canvas_print(struct canvas *canvas) {
	_asm {
		push ebp
		mov ebp, esp
		push ebx
		mov esi, 0
		outer_loop:
		mov ebx, dword ptr[ebp + 8]
		cmp dword ptr[ebx + 4], esi
		je end; test, jestli proiterovaly vsechny radky			
		mov edi, 0
		inner_loop:
		cmp dword ptr[ebx], edi
		je end_inner_loop; test, jestli se vypsaly znaky radku
		mov eax, esi
		mul dword ptr[ebx]
		add eax, edi
		mov edx, 0
		push eax
		shr eax, 2
		mov ecx, eax
		pop eax
		push ecx; ulozeni indexu bytu na zasobnik
		mov ecx, eax
		mov eax, dword ptr[esp]
		shl eax, 2
		sub ecx, eax
		mov eax, ecx
		shl eax, 1; v eax je hodnota odsazeni kodu znaku v bytu
		mov edx, dword ptr[ebx + 8]
		pop ecx
		mov dl, byte ptr[ecx + edx]; byte s kodem znaku
		mov ecx, eax
		shr dl, cl
		and dl, 3; vymaskovani pouze kodu znaku
		push esi
		push edi
		; ulozeni znaku na zasobnik podle kodu
		cmp dl, 0
		je print_d
		cmp dl, 1
		je print_h
		cmp dl, 2
		je print_v
		jmp print_vh
		print_d:
		push empty
		jmp print
		print_v:
		push vsegment
		jmp print
		print_h:
		push hsegment
		jmp print
		print_vh:
		push vhsegment
		print:
		call printf; tisk znaku
		add esp, 4
		pop edi
		pop esi
		inc edi
		jmp inner_loop
		end_inner_loop:
		push esi
		push edi
		push newline
		call printf; tisk znaku noveho radku po jedne iteraci vnitrniho cyklu
		add esp, 4
		pop edi
		pop esi
		inc esi
		jmp outer_loop
		end:
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}

__declspec(naked) struct canvas *canvas_create(unsigned int width, unsigned int height) {
	/*Struktura platna je opet tvorena neznamenkovymi ctyrbytovymi cisly
		  pro sirku a vysku a jednou adresou pro jednorozmerne pole
		  reprezentujici obsah platna. Na to je potreba celkem 12 bytu.*/
	_asm {
		push ebp
		mov ebp, esp
		push ebx
		push 12
		call malloc; alokuje misto pro strukturu platna
		add esp, 4
		mov edx, dword ptr[ebp + 8]
		mov dword ptr[eax], edx; priradi sirku
		mov edx, dword ptr[ebp + 12]
		mov dword ptr[eax + 4], edx; priradi vysku
		mov ebx, eax
		; Nyni je potreba vypocitat delku pole v bytech
		mov eax, edx
		mov edx, dword ptr[ebp + 8]
		mul edx
		shl eax, 1
		add eax, 7
		shr eax, 3
		push eax
		call malloc; alokace pole s delkou(vyska * sirka + 7) / 8
		pop edx
		mov dword ptr[ebx + 8], eax
		mov ecx, 0
		; nasleduje inicializace pole na nuly.
		for_loop:
		cmp edx, ecx
		je end
		mov byte ptr[eax + ecx], 0
		inc ecx
		jmp for_loop
		end:
		mov eax, ebx
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}

__declspec(naked) void canvas_free(struct canvas *canvas) {
	_asm {
		push ebp
		mov ebp, esp
		mov eax, dword ptr[ebp + 8]
		push eax
		mov eax, dword ptr[eax + 8]
		push eax
		call free; dealokace pameti pro pole
		add esp, 4
		call free; dealokace pameti pro strukturu
		add esp, 4
		mov esp, ebp
		pop ebp
		ret
	}
}

__declspec(naked) void canvas_hline(struct canvas *canvas, unsigned int x, unsigned int y, int length) {
	_asm {
		push ebp
		mov ebp, esp
		push ebx
		mov ebx, dword ptr[ebp + 8]
		cmp dword ptr[ebp + 20], 0
		jg greater; test, na kterou stranu se ma kreslit, podle
		; toho se nastavi registr esi urceny k inkrementaci
		jmp lesser
		greater:
		mov esi, 1
		jmp end_cond
		lesser:
		mov esi, -1
		mov eax, dword ptr[ebp + 20]
		imul esi
		mov dword ptr[ebp + 20], eax; nastaveni hodnoty delky na kladnou
		end_cond:
		mov ecx, 0
		for_loop:
		cmp ecx, dword ptr[ebp + 20]; test, jestli ma cara pozadovanou delku
		je end_loop
		; nasleduji testy pro rozpoznani, jestli nedochazi ke kresleni mimo
		; hranice platna
		cmp dword ptr[ebp + 12], ecx
		jb first_and_cond
		jmp or_cond
		first_and_cond:
    	cmp esi, -1
		je end_loop
		or_cond:
		mov eax, dword ptr[ebp + 12]
		add eax, ecx
		add eax, 1
		cmp eax, dword ptr[ebx]
		ja second_and_cond
		jmp cont
		second_and_cond:
		cmp esi, 1
		je end_loop
		cont:
		; vypocet indexu policka platna podle inkrementacni hodnoty v esi
		mov eax, dword ptr[ebx]
		mov edi, dword ptr[ebp + 16]
		mul edi
		cmp esi, 1
		je addition
		sub eax, ecx
		jmp second_cont
		addition:
		add eax, ecx
		second_cont:
		add eax, dword ptr[ebp + 12]; nyni je v eax index policka
		push eax
		shr eax, 2
		pop edx
		push eax
		push edx
		shl eax, 2
		pop edx
		sub edx, eax
		mov eax, edx
		shl eax, 1
		push ecx
		mov ecx, eax;  premisteni odsazeni kodu v bytu do
		mov eax, 1
		shl eax, cl; posun kodu znaku pomoci odsazeni
		pop ecx
		pop edx
		mov edi, dword ptr[ebx + 8]
		or byte ptr[edi + edx], al; prepsani bytu v platne tak, aby reprezentovalo nakresleni znaku
		inc ecx
		jmp for_loop
		end_loop:
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}

__declspec(naked) void canvas_vline(struct canvas *canvas, unsigned int x, unsigned int y, int length) {
	_asm {
		push ebp
		mov ebp, esp
		push ebx
		mov ebx, dword ptr[ebp + 8]
		cmp dword ptr[ebp + 20], 0
		jg greater
		jmp lesser
		greater:
		mov esi, 1
		jmp end_cond
		lesser:
		mov esi, -1
		mov eax, dword ptr[ebp + 20]
		imul esi
		mov dword ptr[ebp + 20], eax
		end_cond:
		mov ecx, 0
		for_loop:
		cmp ecx, dword ptr[ebp + 20]
		je end_loop
		cmp dword ptr[ebp + 16], ecx
		jb first_and_cond
		jmp or_cond
		first_and_cond:
		cmp esi, -1
		je end_loop
		or_cond:
		mov eax, dword ptr[ebp + 16]
		add eax, ecx
		add eax, 1
		cmp eax, dword ptr[ebx + 4]
		jbe cont
		second_and_cond:
		cmp esi, 1
		je end_loop
		cont:
		mov eax, dword ptr[ebx]
		mov edi, dword ptr[ebp + 16]
		cmp esi, 1
		je addition
		sub edi, ecx
		jmp second_cont
		addition:
		add edi, ecx
		second_cont:
		mul edi
		add eax, dword ptr[ebp + 12]
		push eax
		shr eax, 2
		pop edx
		push eax
		push edx
		shl eax, 2
		pop edx
		sub edx, eax
		mov eax, edx
		shl eax, 1
		push ecx
		mov ecx, eax
		mov eax, 2
		shl eax, cl
		pop ecx
		pop edx
		mov edi, dword ptr[ebx + 8]
		or byte ptr[edi + edx], al
		inc ecx
		jmp for_loop
		end_loop:
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}

int main() {
	_asm {
		push ebx
		push 10
		push 20
		call canvas_create
		add esp, 8
		mov ebx, eax

		push 13
		push 2
		push 3
		push ebx
		call canvas_hline
		add esp, 16

		push -11
		push 5
		push 14
		push ebx
		call canvas_hline
		add esp, 16

		push 6
		push 1
		push 4
		push ebx
		call canvas_vline
		add esp, 16

		push -6
		push 6
		push 14
		push ebx
		call canvas_vline
		add esp, 16

		push 20
		push 5
		push 9
		push ebx
		call canvas_vline
		add esp, 16

		push ebx
		call canvas_print
		call canvas_free
		add esp, 4
		mov eax, 0
		pop ebx
	}
}