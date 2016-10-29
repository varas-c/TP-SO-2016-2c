/*
 * deadlock.h
 *
 *  Created on: 14/10/2016
 *      Author: utnso
 */

#ifndef HEADERS_DEADLOCK_H_
#define HEADERS_DEADLOCK_H_

t_pokemon* pokemon_mayor_nivel(t_list* pokemones)
{
	t_pokemon* mas_fuerte = (t_pokemon*)list_get((pokemones),0);
	int i=0;
	for(i=1;i<list_size(pokemones);i++)
	{
		if((((t_pokemon*)list_get(pokemones,i))->level)>(mas_fuerte->level))
		{
			mas_fuerte = (t_pokemon*)list_get(pokemones,i);
		}
	}
	return mas_fuerte;
}

#endif /* HEADERS_DEADLOCK_H_ */
