#includue <stdio.h>

int main () 
{
	
	float mediaVetor = 0;
	float somaVetor = 0;
	float eixoX[20] = {};
	int i;
	
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
	


}


