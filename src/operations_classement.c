/* Fichier classement_liste.c */
/* Contient toutes les fonctions utilis�es pour classer la liste des op�s */

/*     Copyright (C) 2000-2001  C�dric Auger */
/* 			cedric@grisbi.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "en_tete.h"


/* ********************************************************************************************************** */
/* Fonction par d�faut : par ordre de date */
/* ********************************************************************************************************** */

gint classement_liste_par_date ( GtkWidget *liste,
				 GtkCListRow *ligne_1,
				 GtkCListRow *ligne_2 )
{
  struct structure_operation *operation_1;
  struct structure_operation *operation_2;
  gint retour;

  operation_1 = ligne_1 -> data;
  operation_2 = ligne_2 -> data;


  if ( operation_1 == GINT_TO_POINTER ( -1 ) )
    return ( 1 );

  if ( operation_2 == GINT_TO_POINTER ( -1 ) )
    return ( -1 );

  retour = g_date_compare ( operation_1 -> date,
			    operation_2 -> date );

  if ( retour )
    return ( retour );
  else
    return ( operation_1 -> no_operation - operation_2 -> no_operation );

}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
/* classement par no d'op� (donc d'entr�e)  */
/* ********************************************************************************************************** */

gint classement_liste_par_no_ope ( GtkWidget *liste,
				   GtkCListRow *ligne_1,
				   GtkCListRow *ligne_2 )
{
  struct structure_operation *operation_1;
  struct structure_operation *operation_2;
  gint retour;

  operation_1 = ligne_1 -> data;
  operation_2 = ligne_2 -> data;


  if ( operation_1 == GINT_TO_POINTER ( -1 ) )
    return ( 1 );

  if ( operation_2 == GINT_TO_POINTER ( -1 ) )
    return ( -1 );

  return (operation_1 -> no_operation - operation_2 -> no_operation);
}
/* ********************************************************************************************************** */




/* ********************************************************************************************************** */
gint classement_liste_par_tri_courant ( GtkWidget *liste,
					GtkCListRow *ligne_1,
					GtkCListRow *ligne_2 )
{
  struct structure_operation *operation_1;
  struct structure_operation *operation_2;
  gint pos_type_ope_1;
  gint pos_type_ope_2;
  gint buffer;

  operation_1 = ligne_1 -> data;
  operation_2 = ligne_2 -> data;

  if ( operation_1 == GINT_TO_POINTER ( -1 ) )
    return ( 1 );

  if ( operation_2 == GINT_TO_POINTER ( -1 ) )
    return ( -1 );

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  /* si l'op� est n�gative et que le type est neutre et que les types neutres sont s�par�s, on lui */
  /* met la position du type n�gatif */

  if ( operation_1 -> montant < 0
       &&
       ( buffer = g_slist_index ( LISTE_TRI,
				  GINT_TO_POINTER ( -operation_1 -> type_ope ))) != -1 )
    pos_type_ope_1 = buffer;
  else
    pos_type_ope_1 = g_slist_index ( LISTE_TRI,
				     GINT_TO_POINTER ( operation_1 -> type_ope ));

  if ( operation_2 -> montant < 0
       &&
       ( buffer = g_slist_index ( LISTE_TRI,
				  GINT_TO_POINTER ( -operation_2 -> type_ope ))) != -1 )
    pos_type_ope_2 = buffer;
  else
    pos_type_ope_2 = g_slist_index ( LISTE_TRI,
				     GINT_TO_POINTER ( operation_2 -> type_ope ));



  /*   s'ils ont le m�me type, on classe par date */

  if ( pos_type_ope_1 == pos_type_ope_2 )
    return ( g_date_compare ( operation_1->date,
			      operation_2->date ));


  if ( pos_type_ope_1 < pos_type_ope_2 )
    return ( -1 );
  else
    return ( 1 );

}
/* ********************************************************************************************************** */
