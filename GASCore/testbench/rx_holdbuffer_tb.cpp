#include <stdlib.h>
#include "am_rx.hpp"

int main(){

	axis_32a_t input("inputFIFO");
	axis_32a_t output("outputFIFO");

	axis_word_32a_t axi_in_word = {0xDEF};
	uint_1_t release = 1;
	input.write(axi_in_word);
	axi_in_word.data= 0xBEAD;
	input.write(axi_in_word);

	rx_holdbuffer(input, release, output);
	std::cout << "Wrote two words, with release.\n";

	release = 0;
	axi_in_word.data= 0xDEAD;
	input.write(axi_in_word);
	axi_in_word.data= 0xBEEF;
	input.write(axi_in_word);

	rx_holdbuffer(input, release, output);
	std::cout << "Wrote two words, without release.\n";
	axis_word_32a_t axi_out_word;
	int wordCounter = 0;
	while (!output.empty()){
		output.read(axi_out_word);
		std::cout << std::hex << "Initially, read out " << axi_out_word.data << std::endl;
		if(wordCounter == 0 && axi_out_word.data != 0xDEF)
			return 1;
		else if(wordCounter == 1 && axi_out_word.data != 0xBEAD)
			return 2;
		else if(wordCounter > 1)
			return 3;
		wordCounter++;
	}

	if (wordCounter != 2){
		std::cout << std::dec << "Word count: " << wordCounter << std::endl;
		return 11;
	}

	release = 1;
	axi_in_word.data= 0xBEE;
	input.write(axi_in_word);
	axi_in_word.data= 0xDAD;
	input.write(axi_in_word);
	rx_holdbuffer(input, release, output);

	while (!output.empty()){
		output.read(axi_out_word);
		std::cout << std::hex << "Read out " << axi_out_word.data << std::endl;
		if(wordCounter == 2 && axi_out_word.data != 0xDEAD)
			return 4;
		else if(wordCounter == 3 && axi_out_word.data != 0xBEEF)
			return 5;
		else if(wordCounter == 4 && axi_out_word.data != 0xBEE)
			return 6;
		else if(wordCounter == 5 && axi_out_word.data != 0xDAD)
			return 7;
		else if(wordCounter > 5)
			return 8;
		wordCounter++;
		rx_holdbuffer(input, release, output);
	}
	if (wordCounter != 6){
		std::cout << std::dec << "Word count: " << wordCounter << std::endl;
		return 10;
	}

	std::cout << "Success! Read all 6 words\n";
	if(!input.empty()){
		std::cout << "Not all words written? They are: \n";
		while (!input.empty()){
			input.read(axi_out_word);
			std::cout << std::hex << axi_out_word.data << std::endl;
		}
		return 9;
	}

	return 0;

}
