/* fichier qui barre d'outils */
/*           barre_outils.c */

/*     Copyright (C) 2000-2003  C�dric Auger */
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


#include "./xpm/ope_1.xpm"
#include "./xpm/ope_2.xpm"
#include "./xpm/ope_3.xpm"
#include "./xpm/ope_4.xpm"
#include "./xpm/ope_sans_r.xpm"
#include "./xpm/ope_avec_r.xpm"
#include "./xpm/image_fleche_haut.xpm"
#include "./xpm/image_fleche_bas.xpm"
#include "./xpm/liste_0.xpm"
#include "./xpm/liste_1.xpm"
#include "./xpm/liste_2.xpm"
#include "./xpm/liste_3.xpm"
#include "./xpm/comments.xpm"
#include "./xpm/grille.xpm"

#define START_INCLUDE
#include "barre_outils.h"
#include "operations_liste.h"
#include "ventilation.h"
#include "echeancier_liste.h"
#include "operations_formulaire.h"
#include "echeancier_formulaire.h"
#include "menu.h"
#define END_INCLUDE

#define START_STATIC
static void demande_expand_arbre ( GtkWidget *bouton,
			    gint *liste );
#define END_STATIC


/** Used to display/hide comments in scheduler list */
GtkWidget *scheduler_display_hide_comments;

GtkWidget *bouton_affiche_cache_formulaire_echeancier;
GtkWidget *bouton_affiche_commentaire_echeancier;
GtkWidget *fleche_bas_echeancier;
GtkWidget *fleche_haut_echeancier;
GtkWidget *bouton_ope_lignes[4];
GtkWidget *bouton_affiche_r;
GtkWidget *bouton_enleve_r;
GtkWidget *bouton_grille;
GtkWidget *label_proprietes_operations_compte;

/* widgets du bouton pour afficher/cacher le formulaire */

GtkWidget *bouton_affiche_cache_formulaire;
GtkWidget *fleche_haut;
GtkWidget *fleche_bas;




#define START_EXTERN
extern GtkWidget *arbre_categ;
extern GtkWidget *arbre_imputation;
extern GtkWidget *arbre_tiers;
extern GtkWidget *barre_outils;
extern gboolean block_menu_cb ;
extern gint compte_courant;
extern GtkWidget *formulaire;
extern GtkItemFactory *item_factory_menu_general;
extern gint nb_comptes;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern GtkTreeSelection * selection;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkWidget *tree_view_liste_ventilations;
#define END_EXTERN



/*******************************************************************************************/
GtkWidget *creation_barre_outils ( void )
{
    GtkWidget *hbox;
    GtkWidget *separateur;
    GtkWidget *icone;
    GtkWidget *hbox2;


    /*     on utilise le tooltip g�n�ral */

    if ( !tooltips_general_grisbi )
	tooltips_general_grisbi = gtk_tooltips_new ();


    hbox = gtk_hbox_new ( FALSE,
			  5 );


    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( separateur );



    /* bouton affiche / cache le formulaire */

    bouton_affiche_cache_formulaire = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton_affiche_cache_formulaire ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton_affiche_cache_formulaire,
			   _("Display/Hide form"),
			   _("Display/Hide form") );
    gtk_widget_set_usize ( bouton_affiche_cache_formulaire,
			   15,
			   15 );

    hbox2 = gtk_hbox_new ( TRUE,
			   0 );
    gtk_container_add ( GTK_CONTAINER ( bouton_affiche_cache_formulaire ),
			hbox2 );
    gtk_widget_show ( hbox2 );

    fleche_haut = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) image_fleche_haut_xpm ));
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 fleche_haut,
			 FALSE,
			 FALSE,
			 0 );

    fleche_bas = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) image_fleche_bas_xpm ));
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 fleche_bas,
			 FALSE,
			 FALSE,
			 0 );


    if ( etat.formulaire_toujours_affiche )
	gtk_widget_show ( fleche_bas );
    else
	gtk_widget_show ( fleche_haut );

    gtk_signal_connect ( GTK_OBJECT ( bouton_affiche_cache_formulaire ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( affiche_cache_le_formulaire ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_affiche_cache_formulaire,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_cache_formulaire );


    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( separateur );


    /* bouton op�rations 1 ligne */

    bouton_ope_lignes[0] = gtk_radio_button_new ( FALSE );
    gtk_toggle_button_set_mode ( GTK_TOGGLE_BUTTON ( bouton_ope_lignes[0]),
				 FALSE );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_lignes[0] ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton_ope_lignes[0],
			   _("One line per transaction"),
			   _("One line per transaction") );

    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) ope_1_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton_ope_lignes[0] ),
			icone );
    gtk_widget_set_usize ( bouton_ope_lignes[0],
			   15,
			   15 );
    g_signal_connect_swapped ( GTK_OBJECT ( bouton_ope_lignes[0] ),
			       "toggled",
			       GTK_SIGNAL_FUNC ( change_aspect_liste ),
			       GINT_TO_POINTER ( 1 ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_ope_lignes[0],
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton_ope_lignes[0] );


    /* bouton op�rations 2 lignes */

    bouton_ope_lignes[1] = gtk_radio_button_new_from_widget ( GTK_RADIO_BUTTON ( bouton_ope_lignes[0] ));
    gtk_toggle_button_set_mode ( GTK_TOGGLE_BUTTON ( bouton_ope_lignes[1]),
				 FALSE );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_lignes[1] ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton_ope_lignes[1],
			   _("Two lines per transaction"),
			   _("Two lines per transaction") );

    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) ope_2_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton_ope_lignes[1] ),
			icone );
    g_signal_connect_swapped ( GTK_OBJECT ( bouton_ope_lignes[1] ),
			       "toggled",
			       GTK_SIGNAL_FUNC ( change_aspect_liste ),
			       GINT_TO_POINTER ( 2 ) );
    gtk_widget_set_usize ( bouton_ope_lignes[1],
			   15,
			   15 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_ope_lignes[1],
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton_ope_lignes[1] );




    /* bouton op�rations 3 lignes */

    bouton_ope_lignes[2] = gtk_radio_button_new_from_widget ( GTK_RADIO_BUTTON ( bouton_ope_lignes[0] ));
    gtk_toggle_button_set_mode ( GTK_TOGGLE_BUTTON ( bouton_ope_lignes[2]),
				 FALSE );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_lignes[2] ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton_ope_lignes[2],
			   _("Three lines per transaction"),
			   _("Three lines per transaction") );

    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) ope_3_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton_ope_lignes[2] ),
			icone );
    g_signal_connect_swapped ( GTK_OBJECT ( bouton_ope_lignes[2] ),
			       "toggled",
			       GTK_SIGNAL_FUNC ( change_aspect_liste ),
			       GINT_TO_POINTER ( 3 ) );
    gtk_widget_set_usize ( bouton_ope_lignes[2],
			   15,
			   15 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_ope_lignes[2],
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton_ope_lignes[2] );


    /* bouton op�rations 4 lignes */

    bouton_ope_lignes[3] = gtk_radio_button_new_from_widget ( GTK_RADIO_BUTTON ( bouton_ope_lignes[0] ));
    gtk_toggle_button_set_mode ( GTK_TOGGLE_BUTTON ( bouton_ope_lignes[3]),
				 FALSE );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_lignes[3] ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton_ope_lignes[3],
			   _("Four lines per transaction"),
			   _("Four lines per transaction") );

    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) ope_4_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton_ope_lignes[3] ),
			icone );
    g_signal_connect_swapped ( GTK_OBJECT ( bouton_ope_lignes[3] ),
			       "toggled",
			       GTK_SIGNAL_FUNC ( change_aspect_liste ),
			       GINT_TO_POINTER ( 4 ) );
    gtk_widget_set_usize ( bouton_ope_lignes[3],
			   15,
			   15 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_ope_lignes[3],
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton_ope_lignes[3] );


    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( separateur );


    /* bouton affiche op�rations relev�es */

    bouton_affiche_r = gtk_radio_button_new ( NULL);
    gtk_toggle_button_set_mode ( GTK_TOGGLE_BUTTON ( bouton_affiche_r ),
				 FALSE );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_affiche_r ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton_affiche_r,
			   _("Display reconciled transactions"),
			   _("Display reconciled transactions") );
    gtk_widget_set_usize ( bouton_affiche_r,
			   15,
			   15 );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) (const gchar **) ope_avec_r ));
    gtk_container_add ( GTK_CONTAINER ( bouton_affiche_r ),
			icone );
    g_signal_connect_swapped ( GTK_OBJECT ( bouton_affiche_r ),
			       "toggled",
			       GTK_SIGNAL_FUNC ( change_aspect_liste ),
			       GINT_TO_POINTER ( 5 ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_affiche_r,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton_affiche_r );


    /* bouton efface op�rations relev�es */

    bouton_enleve_r = gtk_radio_button_new_from_widget ( GTK_RADIO_BUTTON ( bouton_affiche_r ));
    gtk_toggle_button_set_mode ( GTK_TOGGLE_BUTTON (bouton_enleve_r ),
				 FALSE );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_enleve_r ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton_enleve_r,
			   _("Mask reconciled transactions"),
			   _("Mask reconciled transactions") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) ope_sans_r ));
    gtk_container_add ( GTK_CONTAINER ( bouton_enleve_r ),
			icone );
    gtk_widget_set_usize ( bouton_enleve_r,
			   15,
			   15 );
    g_signal_connect_swapped ( GTK_OBJECT ( bouton_enleve_r ),
			       "toggled",
			       GTK_SIGNAL_FUNC ( change_aspect_liste ),
			       GINT_TO_POINTER ( 6 ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_enleve_r,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton_enleve_r );


    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( separateur );

    /*     bouton affiche/masque la grille des op�rations */

    bouton_grille = gtk_check_button_new ();
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (bouton_grille ),
				   etat.affichage_grille );
    gtk_toggle_button_set_mode ( GTK_TOGGLE_BUTTON (bouton_grille ),
				 FALSE  );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_grille ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton_grille,
			   _("Display grid"),
			   _("Display grid") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) grille_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton_grille ),
			icone );
    gtk_widget_set_usize ( bouton_grille,
			   15,
			   15 );
    g_signal_connect_swapped ( GTK_OBJECT ( bouton_grille ),
			       "toggled",
			       GTK_SIGNAL_FUNC ( change_aspect_liste ),
			       NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_grille,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton_grille );





    label_proprietes_operations_compte = gtk_label_new (_("Account transactions"));
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       label_proprietes_operations_compte,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( label_proprietes_operations_compte );

    return ( hbox );
}
/*******************************************************************************************/



/****************************************************************************************************/
gboolean change_aspect_liste ( gint demande )
{
    gint i;
    GtkWidget * widget;

    p_tab_nom_de_compte_variable=p_tab_nom_de_compte + compte_courant;

/* FIXME : d�s que gtk 2.4 � la maison, utiliser le gtk_tree_model_filter */

    block_menu_cb = TRUE;

    switch ( demande )
    {
	case 0:
	    /* 	    changement de l'affichage de la grille */

/* 	    etat.affichage_grille = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_grille )); */
	  etat.affichage_grille = ! etat.affichage_grille;

	    if ( etat.affichage_grille )
	    {
		/* 		on affiche les grilles */

		g_signal_connect_after ( G_OBJECT ( tree_view_liste_ventilations ),
					 "expose-event",
					 G_CALLBACK ( affichage_traits_liste_ventilation ),
					 NULL );

		for ( i=0 ; i<nb_comptes ; i++ )
		{
		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		    g_signal_connect_after ( G_OBJECT ( TREE_VIEW_LISTE_OPERATIONS ),
					     "expose-event",
					     G_CALLBACK ( affichage_traits_liste_operation ),
					     NULL );
		}
	    }
	    else
	    {
		g_signal_handlers_disconnect_by_func ( G_OBJECT ( tree_view_liste_ventilations ),
						       G_CALLBACK ( affichage_traits_liste_ventilation ),
						       NULL );
		for ( i=0 ; i<nb_comptes ; i++ )
		{
		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		    g_signal_handlers_disconnect_by_func ( G_OBJECT ( TREE_VIEW_LISTE_OPERATIONS ),
							   G_CALLBACK ( affichage_traits_liste_operation ),
							   NULL );
		}
	    }
	    p_tab_nom_de_compte_variable=p_tab_nom_de_compte + compte_courant;
	    gtk_widget_queue_draw ( TREE_VIEW_LISTE_OPERATIONS );

	    block_menu_cb = TRUE;
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 menu_name(_("View"), _("Show grid"), NULL) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), 
					    etat.affichage_grille );
	    block_menu_cb = FALSE;

	    break;

	/* 	1, 2, 3 et 4 sont les nb de lignes qu'on demande � afficher */

	case 1 :
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 menu_name(_("View"), _("Show one line per transaction"), NULL) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    mise_a_jour_affichage_lignes ( demande );
	    break;
	case 2 :
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 menu_name(_("View"), _("Show two lines per transaction"), NULL) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    mise_a_jour_affichage_lignes ( demande );
	    break;
	case 3 :
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 menu_name(_("View"), _("Show three lines per transaction"), NULL) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    mise_a_jour_affichage_lignes ( demande );
	    break;
	case 4 :
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 menu_name(_("View"), _("Show four lines per transaction"), NULL) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    mise_a_jour_affichage_lignes ( demande );
	    break;

	case 5 :

	    /* ope avec r */

	    mise_a_jour_affichage_r ( 1 );

	    block_menu_cb = TRUE;
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 menu_name(_("View"), _("Show reconciled transactions"), NULL) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    block_menu_cb = FALSE;

	    break;

	case 6 :

	    /* ope sans r */

	    mise_a_jour_affichage_r ( 0 );

	    block_menu_cb = TRUE;
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 menu_name(_("View"), _("Show reconciled transactions"), NULL) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), FALSE );
	    block_menu_cb = FALSE;

	    break;
    }

    block_menu_cb = FALSE;

    return ( TRUE );
}
/* ***************************************************************************************************** */




/*******************************************************************************************/
GtkWidget *creation_barre_outils_echeancier ( void )
{
    GtkWidget *hbox;
    GtkWidget *separateur;
    GtkWidget *hbox2;


    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_widget_show ( hbox );

    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );



    /* bouton affiche / cache le formulaire */

    bouton_affiche_cache_formulaire_echeancier = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton_affiche_cache_formulaire_echeancier ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton_affiche_cache_formulaire_echeancier,
			   _("Display/hide form"),
			   _("Display/hide form") );
    gtk_widget_set_usize ( bouton_affiche_cache_formulaire_echeancier,
			   15,
			   15 );

    hbox2 = gtk_hbox_new ( TRUE,
			   0 );
    gtk_container_add ( GTK_CONTAINER ( bouton_affiche_cache_formulaire_echeancier ),
			hbox2 );
    gtk_widget_show ( hbox2 );

    fleche_haut_echeancier = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) image_fleche_haut_xpm ));
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 fleche_haut_echeancier,
			 FALSE,
			 FALSE,
			 0 );

    fleche_bas_echeancier = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) image_fleche_bas_xpm ));
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 fleche_bas_echeancier,
			 FALSE,
			 FALSE,
			 0 );


    if ( etat.formulaire_echeancier_toujours_affiche )
	gtk_widget_show ( fleche_bas_echeancier );
    else
	gtk_widget_show ( fleche_haut_echeancier );

    gtk_signal_connect ( GTK_OBJECT ( bouton_affiche_cache_formulaire_echeancier ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( affiche_cache_le_formulaire_echeancier ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_affiche_cache_formulaire_echeancier,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_cache_formulaire_echeancier );


    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );

    /* dOm : ajout commutateur d'affichage de commentaires */
    /* bouton affiche / cache le commentaire dans la liste de l'echeancier */
    scheduler_display_hide_comments = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) comments_xpm ));
    bouton_affiche_commentaire_echeancier = gtk_button_new ();
    gtk_widget_show ( scheduler_display_hide_comments );
    gtk_container_add ( GTK_CONTAINER ( bouton_affiche_commentaire_echeancier ),
			scheduler_display_hide_comments );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_affiche_commentaire_echeancier ),
			    GTK_RELIEF_NONE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
			   bouton_affiche_commentaire_echeancier ,
			   _("Display/hide comments"),
			   _("Display/hide comments") );
    gtk_widget_set_usize ( bouton_affiche_commentaire_echeancier ,
			   16, 16 );
    gtk_signal_connect ( GTK_OBJECT ( bouton_affiche_commentaire_echeancier ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( affiche_cache_commentaire_echeancier ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_affiche_commentaire_echeancier,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_commentaire_echeancier );

    return ( hbox );
}
/*******************************************************************************************/




/*******************************************************************************************/
GtkWidget *creation_barre_outils_tiers ( void )
{
    GtkWidget *hbox;
    GtkWidget *separateur;
    GtkWidget *icone;
    GtkWidget *bouton;

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_widget_show ( hbox );

    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    /* bouton fermeture de l'arbre */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 0 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Close tree"),
			   _("Close tree") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_0_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    /* bouton ouverture de l'arbre niveau 1 */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 1 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Display accounts"),
			   _("Display accounts") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_1_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    /* bouton ouverture de l'arbre niveau 2 */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 2 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Display transactions"),
			   _("Display transactions") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_2_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    return ( hbox );
}
/*******************************************************************************************/





/*******************************************************************************************/
GtkWidget *creation_barre_outils_categ ( void )
{
    GtkWidget *hbox;
    GtkWidget *separateur;
    GtkWidget *icone;
    GtkWidget *bouton;

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_widget_show ( hbox );

    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    /* bouton fermeture de l'arbre */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 0 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Close tree"),
			   _("Close tree") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_0_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 GINT_TO_POINTER (1));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    /* bouton ouverture de l'arbre niveau 1 */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 1 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Display sub-divisions"),
			   _("Display sub-divisions") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_1_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 GINT_TO_POINTER (1));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    /* bouton ouverture de l'arbre niveau 2 */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 2 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Display accounts"),
			   _("Display accounts") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_2_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 GINT_TO_POINTER (1));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    /* bouton ouverture de l'arbre niveau 3 */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 3 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Display transactions"),
			   _("Display transactions") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_3_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 GINT_TO_POINTER (1));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    return ( hbox );
}
/*******************************************************************************************/






/*******************************************************************************************/
GtkWidget *creation_barre_outils_imputation ( void )
{
    GtkWidget *hbox;
    GtkWidget *separateur;
    GtkWidget *icone;
    GtkWidget *bouton;

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_widget_show ( hbox );

    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    /* bouton fermeture de l'arbre */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 0 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Close tree"),
			   _("Close tree") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_0_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 GINT_TO_POINTER (2));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    /* bouton ouverture de l'arbre niveau 1 */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 1 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Display sub-divisions"),
			   _("Display sub-divisions") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_1_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 GINT_TO_POINTER (2));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    /* bouton ouverture de l'arbre niveau 2 */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 2 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Display accounts"),
			   _("Display accounts") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_2_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 GINT_TO_POINTER (2));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    /* bouton ouverture de l'arbre niveau 3 */

    bouton = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_object_set_data ( GTK_OBJECT ( bouton ),
			  "profondeur",
			  GINT_TO_POINTER ( 3 ));
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi  ),
			   bouton,
			   _("Display transactions"),
			   _("Display transactions") );
    icone = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) liste_3_xpm ));
    gtk_container_add ( GTK_CONTAINER ( bouton ),
			icone );
    gtk_widget_set_usize ( bouton,
			   15,
			   15 );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( demande_expand_arbre ),
			 GINT_TO_POINTER (2));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( bouton );

    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    return ( hbox );
}
/*******************************************************************************************/


/*******************************************************************************************/
/* �tend l'arbre donn� en argument en fonction du bouton cliqué (profondeur contenue */
/* dans le bouton) */
/*******************************************************************************************/

void demande_expand_arbre ( GtkWidget *bouton,
			    gint *liste )
{
    GtkWidget *ctree;
    gint i;
    GtkCTreeNode *noeud_selectionne;

    if ( liste )
    {
	if ( GPOINTER_TO_INT ( liste ) == 1 )
	    ctree = arbre_categ;
	else
	    ctree = arbre_imputation;
    }
    else
	ctree = arbre_tiers;

    gtk_clist_freeze ( GTK_CLIST ( ctree ));

    /* on doit faire �a �tage par �tage car il y a des ajouts � chaque ouverture de noeud */

    /*   r�cup�re le noeud s�lectionn�, s'il n'y en a aucun, fera tout l'arbre */

    noeud_selectionne = NULL;

    if ( GTK_CLIST ( ctree ) -> selection )
	noeud_selectionne = GTK_CLIST ( ctree ) -> selection -> data;

    gtk_ctree_collapse_recursive ( GTK_CTREE ( ctree ),
				   noeud_selectionne );

    for ( i=0 ; i < GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( bouton ),
							    "profondeur" )) ; i++ )
	gtk_ctree_expand_to_depth ( GTK_CTREE ( ctree ),
				    noeud_selectionne,
				    i + 1);
    gtk_clist_thaw ( GTK_CLIST ( ctree ));
}
/*******************************************************************************************/


/*******************************************************************************************/
/* cette fonction met les boutons du nb lignes par op� et de l'affichage de R en fonction du compte */
/* envoy� en argument */
/*******************************************************************************************/

void mise_a_jour_boutons_caract_liste ( gint no_compte )
{
    p_tab_nom_de_compte_variable=p_tab_nom_de_compte + no_compte;

    /*     on veut juste mettre les boutons � jour, sans redessiner la liste */
    /*     on bloque donc les appels aux fonctions */

    g_signal_handlers_block_by_func ( G_OBJECT ( bouton_ope_lignes[0] ),
				      G_CALLBACK ( change_aspect_liste ),
				      GINT_TO_POINTER (1));
    g_signal_handlers_block_by_func ( G_OBJECT ( bouton_ope_lignes[1] ),
				      G_CALLBACK ( change_aspect_liste ),
				      GINT_TO_POINTER (2));
    g_signal_handlers_block_by_func ( G_OBJECT ( bouton_ope_lignes[2] ),
				      G_CALLBACK ( change_aspect_liste ),
				      GINT_TO_POINTER (3));
    g_signal_handlers_block_by_func ( G_OBJECT ( bouton_ope_lignes[3] ),
				      G_CALLBACK ( change_aspect_liste ),
				      GINT_TO_POINTER (4));
    switch ( NB_LIGNES_OPE )
    {
	case 1:
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_lignes[0] ),
					   TRUE );
	    break;
	case 2:
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_lignes[1] ),
					   TRUE );
	    break;
	case 3:
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_lignes[2] ),
					   TRUE );
	    break;
	case 4:
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_lignes[3] ),
					   TRUE );
	    break;
    }
    g_signal_handlers_unblock_by_func ( G_OBJECT ( bouton_ope_lignes[0] ),
					G_CALLBACK ( change_aspect_liste ),
					GINT_TO_POINTER (1));
    g_signal_handlers_unblock_by_func ( G_OBJECT ( bouton_ope_lignes[1] ),
					G_CALLBACK ( change_aspect_liste ),
					GINT_TO_POINTER (2));
    g_signal_handlers_unblock_by_func ( G_OBJECT ( bouton_ope_lignes[2] ),
					G_CALLBACK ( change_aspect_liste ),
					GINT_TO_POINTER (3));
    g_signal_handlers_unblock_by_func ( G_OBJECT ( bouton_ope_lignes[3] ),
					G_CALLBACK ( change_aspect_liste ),
					GINT_TO_POINTER (4));


    /* on met maintenant le bouton r ou pas r */

    g_signal_handlers_block_by_func ( G_OBJECT ( bouton_affiche_r ),
				      G_CALLBACK ( change_aspect_liste ),
				      GINT_TO_POINTER (5));
    g_signal_handlers_block_by_func ( G_OBJECT ( bouton_enleve_r ),
				      G_CALLBACK ( change_aspect_liste ),
				      GINT_TO_POINTER (6));
    if ( AFFICHAGE_R )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_r ),
				       TRUE );
    else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_enleve_r ),
				       TRUE );

    g_signal_handlers_unblock_by_func ( G_OBJECT ( bouton_affiche_r ),
					G_CALLBACK ( change_aspect_liste ),
					GINT_TO_POINTER (5));
    g_signal_handlers_unblock_by_func ( G_OBJECT ( bouton_enleve_r ),
					G_CALLBACK ( change_aspect_liste ),
					GINT_TO_POINTER (6));

    /* On met maintenant le bouton grille ou pas */
    g_signal_handlers_block_by_func ( G_OBJECT ( bouton_grille ),
				      G_CALLBACK ( change_aspect_liste ),
				      GINT_TO_POINTER (0));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_grille ), 
				   etat.affichage_grille );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( bouton_grille ),
					G_CALLBACK ( change_aspect_liste ),
					GINT_TO_POINTER (0));

}
/*******************************************************************************************/
