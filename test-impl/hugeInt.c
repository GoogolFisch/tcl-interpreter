
// use -lgmp

#include<gmp.h>
#include<stdio.h>
#include<stdint.h>


int main(){
	mpz_t va,vb,vc;
	mpz_inits(va,vb,vc,NULL);
	mpz_set_ui(vb,1);
	// mpz_set_str(dst, char*,base)

	for(int32_t i = 0;i < 200;i++){
		mpz_add(vc,vb,va);
		mpz_swap(va,vb);
		mpz_swap(vc,vb);
		gmp_printf("%#50Zd\n",va);
	}

	mpz_clears(va,vb,vc,NULL); /*
	mpz_clear(va);
	mpz_clear(vb);
	mpz_clear(vc);
	// */
	return 0;
}
