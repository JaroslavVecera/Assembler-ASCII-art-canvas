#include <stdio.h>
#include <stdlib.h>


struct canvas {
	unsigned int width;
	unsigned int height;
	char *content;
};

struct canvas *canvas_create(unsigned int width, unsigned int height) {
	struct canvas *res = (struct canvas*)malloc(sizeof(struct canvas));

	res->width = width;
	res->height = height;
	unsigned int fields_count = width * height;
	unsigned int bits_needed = fields_count * 2; //kazdé policko je reprezentovano dvema bity
	unsigned int bytes_needed = (bits_needed + 7) / 8; //(a + b - 1) / b nahrazuje vypocet ceil(a / b)
	res->content = (char*)malloc(bytes_needed);
	for (int i = 0; i < bytes_needed; i++) {
		res->content[i] = 0; //vynulovani pole
	}

	return res;
}

void canvas_hline(struct canvas *canvas, unsigned int x, unsigned int y, int length) {
	int inc = 1;
	/*Nejprve nastaví hodnotu promìnné inc na 1, nebo -1 podle
	  toho, jestli bude kreslit caru doprava, ci doleva. Pote nahradi
	  hodnotu length jeji absolutni hodnotou.*/
	if (length < 0) {
		inc = -1;
		length *= -1;
	}
	for (int i = 0; i < length; i++) {
		/*Hodnota i se v obou pripadech orientace cary zvyšuje. V pripade kresleni doleva
		  je tedy potreba kontrolovat, jestli neni i > x, potom by totiz dochazelo ke kresleni
		  za hranu canvasu. V pripade kresleni doprava je potreba kontrolovat jestli neni
		  souradnice policka vyssi nez sirka canvasu.*/
		if ((x < i && inc == -1) || (inc == 1 && x + i + 1 > canvas->width))
			break;
		unsigned int field_index;
		/*Prepocet souradnic policka do indexu policka canvasu reprezentovaného jednorozmernym polem*/
		if (inc == 1)
			field_index = x + i + y * canvas->width;
		else
			field_index = x - i + y * canvas->width;
		unsigned int byte_index = field_index / 4;
		/*kod znaku canvasu je v danem bytu posunut o offset bitu doleva*/
		unsigned int offset = (field_index - byte_index * 4) * 2;
		char byte = 1 << offset;
		/*operace |= provedená na nejakem bytu canvasu a bytu ve tvaru 00(01)0000 provede zmenu jen
		  v miste dvoubitu 01. Pokud je na odpovídajícím místì prvního operandu hodnota 00 ("."),
		  zmeni se kod na 01 ("-"), pokud se tam nachazi 01 nebo 11 ("+") , kod se nezmeni. V pripade kodu
		  10 ("|") se zmeni na 11. To je pozadovane chovani funkce.*/
		canvas->content[byte_index] |= byte;
	}
}

void canvas_vline(struct canvas *canvas, unsigned int x, unsigned int y, int length) {
	/*Je vytvorena analogicky k canvas_hline. Meni se souradnice a kod znaku.*/
	int inc = 1;
	if (length < 0) {
		inc = -1;
		length *= -1;
	}
	for (int i = 0; i < length; i++) {
		if ((y < i && inc == -1) || (inc == 1 && y + i + 1 > canvas->height))
			break;
		unsigned int field_index;
		if (inc == 1)
			field_index = x + (y + i) * canvas->width;
		else
			field_index = x + (y - i) * canvas->width;
		unsigned int byte_index = field_index / 4;
		unsigned int offset = (field_index - byte_index * 4) * 2;
		char byte = 2 << offset;
		canvas->content[byte_index] |= byte;
	}
}

void canvas_print(struct canvas *canvas) {
	for (unsigned int i = 0; i < canvas->height; i++) {
		for (unsigned int j = 0; j < canvas->width; j++) {
			/*Pro kazde policko najde obdobne jako v canvas_hline a canvas_vline
			  index policka v jednorozmerne reprezentaci canvasu, index bytu
			  a odsazeni bitu v tomto bytu. Pote cely byte posune tak, aby
			  se dvoubitovy kod nacházel co nejvice vpravo. Ostatni bity oreze
			  za pomoci masky 00000011 (a operace &). Potom podle kodu vytiskne odpovidajici znak.*/
			unsigned int field_index = i * canvas->width + j;
			unsigned int byte_index = field_index / 4;
			unsigned int offset = (field_index - byte_index * 4) * 2;
			unsigned char byte = canvas->content[byte_index];
			unsigned char field_code = (byte >> offset) & (unsigned char)3;
			switch (field_code) {
			case 0:
				printf(".");
				break;
			case 1:
				printf("-");
				break;
			case 2:
				printf("|");
				break;
			case 3:
				printf("+");
				break;
			}
		}
		/*Pruchod vnitrniho cyklu reprezentuje jeden radek canvasu. Vytiskne se
		  tedy znak noveho radku.*/
		printf("\n");
	}
}

void canvas_free(struct canvas *canvas)
{
	/*Je potreba nejprve dealokovat pamet pole canvas->content.*/
	free(canvas->content);
	free(canvas);
}

int main() {
	struct canvas *c = canvas_create_a(20, 10);
	canvas_hline(c, 3, 2, 13);
	canvas_hline(c, 14, 5, -11);
	canvas_vline(c, 4, 1, 6);
	canvas_vline(c, 14, 6, -6);
	canvas_vline(c, 9, 5, 20);
	canvas_print(c);
	canvas_free(c);
	return 0;
}