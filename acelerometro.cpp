#includue <stdio.h>

int i;	
float mediaVetor = 0;
float somaVetor = 0;
float eixoX[20] = {};
float eixoY[20] = {};
float eixoZ[20] = {};
float eixo[20] = {};
float eixo;
float leituraInicial = 0;
float leituraAtual = 0;
float variacao = 0;
	



//Função que retornará a diferença de duas leituras de um eixo qualquer escolhido
float variacaoEixo(float eixo)
{
	//Guarda as 20 leituras do eixo  em um vetor
	for (i = 0; i < 20; i++)
	{
	eixo[i] = analogRead(eixo);
	}
	//Soma as 20 leituras do eixo 
	for (i = 0; i < 20; i++){
	somaVetor = eixo[i] + somaVetor;	
	}
	//Faz a media das 20 leituras do eixo 	
	mediaVetor = somaVetor/20;
	mediaVetor = valorInicial;
	// aguarda 1 segundo para uma nova leitura
	delay(1000); 
	for (i = 0; i < 20; i++)
	{
	eixo[i] = analogRead(eixo);
	}
	for (i = 0; i < 20; i++)
	{
	somaVetor = eixo[i] + somaVetor;	
	}	
	mediaVetor = somaVetor/20;
	mediaVetor = valorAtual;
	variacao = valorAtual - ValorInicial;
	return(variacao);	
}

int main (void) 
{

variacao(eixoX);
	
	
	
}
	
	
	
	
//Verifica se os demais eixos estão não estão variando, impedindo que ocorra uma leitura errada da densidade caso alguem balance ou 
//toque no recipiente do mosto
	

	


}


