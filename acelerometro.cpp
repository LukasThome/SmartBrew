#includue <stdio.h>

int main () 
{
	
	float mediaVetor = 0;
	float somaVetor = 0;
	float eixoX[20] = {};
	float eixoY[20] = {};
	float eixoZ[20] = {};
	int i;
	float eixo;
	float leituraInicial = 0;
	float leituraAtual = 0;
	float variacao = 0;
	
	
	//Função que retornará a diferença de duas leituras de um eixo qualquer escolhido
	float variacaoEixo(float eixo)
	{
		//Guarda as 20 leituras do eixo x em um vetor
		for (i = 0; i < 20; i++)
		{
		eixoX[i] = analogRead(X);
		}
		//Soma as 20 leituras do eixo X
		for (i = 0; i < 20; i++){
		somaVetor = eixoX[i] + somaVetor;	
		}
		//Faz a media das 20 leituras do eixo 	
		mediaVetor = somaVetor/20;
		
		mediaVetor = valorInicial;
		
		delay(1000);
		
		//Guarda as 20 leituras do eixo x em um vetor
		for (i = 0; i < 20; i++)
		{
		eixoX[i] = analogRead(X);
		}
		//Soma as 20 leituras do eixo X
		for (i = 0; i < 20; i++){
		somaVetor = eixoX[i] + somaVetor;	
		}
		//Faz a media das 20 leituras do eixo 	
		mediaVetor = somaVetor/20;
		
		mediaVetor = valorAtual;
		
		variacao = valorAtual - ValorInicial;
		
		return(variacao);
		
	
	}

//Verifica se os demais eixos estão não estão variando, impedindo que ocorra uma leitura errada da densidade caso alguem balance ou 
//toque no recipiente do mosto
	

	


}

