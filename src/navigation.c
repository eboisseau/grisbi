/* ************************************************************************** */
/*     Copyright (C)	2005-2006 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#include "include.h"

/*START_INCLUDE*/
#include "navigation.h"
#include "equilibrage.h"
#include "echeancier_infos.h"
#include "erreur.h"
#include "gsb_data_account.h"
#include "operations_comptes.h"
#include "gsb_data_currency.h"
#include "gsb_data_report.h"
#include "gsb_plugins.h"
#include "fenetre_principale.h"
#include "etats_onglet.h"
#include "menu.h"
#include "gsb_real.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "comptes_gestion.h"
#include "gsb_file_config.h"
#include "navigation.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void create_report_list ( GtkTreeModel * model, GtkTreeIter * reports_iter );
static gboolean gsb_gui_navigation_check_key_press ( GtkWidget *tree_view,
					      GdkEventKey *ev,
					      GtkTreeModel *model );
static gboolean gsb_gui_navigation_line_visible_p ( GtkTreeModel * model, GtkTreeIter * iter,
					     gpointer data );
static  gboolean gsb_gui_navigation_remove_account_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data );
static  gboolean gsb_gui_navigation_remove_report_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data );
static gboolean gsb_gui_navigation_select_line ( GtkTreeSelection * selection,
					  GtkTreeModel * model );
static void gsb_gui_navigation_set_selection_branch ( GtkTreeSelection * selection, 
					       GtkTreeIter * iter, gint page, 
					       gint account_nb, gpointer report );
static void gsb_gui_navigation_update_account_iter ( GtkTreeModel * model, 
					      GtkTreeIter * account_iter,
					      gint account_nb );
static  gboolean gsb_gui_navigation_update_account_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data );
static gboolean gsb_gui_navigation_update_notebook ( GtkTreeSelection * selection,
					      GtkTreeModel * model );
static void gsb_gui_navigation_update_report_iter ( GtkTreeModel * model, 
					     GtkTreeIter * report_iter,
					     gint report_number );
static  gboolean gsb_gui_navigation_update_report_iterator ( GtkTreeModel * tree_model, 
							    GtkTreePath *path, 
							    GtkTreeIter *iter, 
							    gpointer data );
static void navigation_change_account_order ( GtkTreeModel * model, gint orig, gint dest );
static void navigation_change_report_order ( GtkTreeModel * model, gint orig, gint dest );
static gboolean navigation_sort_column ( GtkTreeModel * model, 
				  GtkTreeIter * a, GtkTreeIter * b, 
				  gpointer user_data );
static gboolean navigation_tree_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
					 GtkSelectionData * selection_data );
/*END_STATIC*/


/*START_EXTERN*/
extern gint compte_courant_onglet;
extern GtkTreeSelection * selection;
extern GSList *sort_accounts;
extern gchar *titre_fichier;
extern GtkWidget *tree_view;
/*END_EXTERN*/


/** Navigation tree view. */
GtkWidget * navigation_tree_view;

/** Model of the navigation tree. */
GtkTreeModel * navigation_model;

/** Widget that hold the scheduler calendar. */
GtkWidget * scheduler_calendar;

/** Widget that hold all reconciliation widgets. */
GtkWidget * reconcile_panel;




/**
 * Create the navigation pane on the left of the GUI.  It contains
 * account list as well as shortcuts.
 *
 * \return The newly allocated pane.
 */
GtkWidget * create_navigation_pane ( void )
{
    GtkWidget * sw, *vbox;
    GdkPixbuf * pixbuf;
    GtkTreeIter iter, account_iter, reports_iter;
    GtkCellRenderer * renderer;
    GtkTreeDragDestIface * navigation_dst_iface;
    GtkTreeDragSourceIface * navigation_src_iface;
    GtkTreeViewColumn * column;
    static GtkTargetEntry row_targets[] = {
	{ "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
    };

    vbox = gtk_vbox_new ( FALSE, 6 );

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER,
				    GTK_POLICY_AUTOMATIC);

    /* Create the view */
    navigation_tree_view = gtk_tree_view_new ();
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW(navigation_tree_view), FALSE );
    gtk_container_add ( GTK_CONTAINER(sw), navigation_tree_view );

    navigation_model = gtk_tree_store_new ( NAVIGATION_TOTAL, 
					    GDK_TYPE_PIXBUF,
					    G_TYPE_BOOLEAN, G_TYPE_STRING, 
					    G_TYPE_INT, G_TYPE_INT, 
					    G_TYPE_INT, G_TYPE_INT,
					    G_TYPE_INT );

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(navigation_model),
					   NAVIGATION_PAGE, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(navigation_model), 
				      NAVIGATION_PAGE, navigation_sort_column,
				      NULL, NULL );

    /* Enable drag & drop */
    gtk_tree_view_enable_model_drag_source ( GTK_TREE_VIEW(navigation_tree_view),
					     GDK_BUTTON1_MASK, row_targets, 1,
					     GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_enable_model_drag_dest ( GTK_TREE_VIEW(navigation_tree_view), row_targets,
					   1, GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_set_reorderable ( GTK_TREE_VIEW(navigation_tree_view), TRUE );
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view)),
				  GTK_SELECTION_SINGLE );
    gtk_tree_view_set_model ( GTK_TREE_VIEW(navigation_tree_view), 
			      GTK_TREE_MODEL(navigation_model) );

    /* Handle drag & drop */
    navigation_dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE (navigation_model);
    if ( navigation_dst_iface )
    {
	navigation_dst_iface -> drag_data_received = &navigation_drag_data_received;
	navigation_dst_iface -> row_drop_possible = &navigation_row_drop_possible;
    }

    navigation_src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE (navigation_model);
    if ( navigation_src_iface )
    {
	gtk_selection_add_target (navigation_tree_view,
				  GDK_SELECTION_PRIMARY,
				  GDK_SELECTION_TYPE_ATOM,
				  1);
	navigation_src_iface -> drag_data_get = &navigation_tree_drag_data_get;
    }

    /* check the keyboard before all, if we need to move other things that the navigation
     * tree view (for example, up and down on transactions list) */
    g_signal_connect ( navigation_tree_view,
		       "key-press-event", 
		       G_CALLBACK (gsb_gui_navigation_check_key_press),
		       navigation_model  );

    g_signal_connect_after (gtk_tree_view_get_selection (GTK_TREE_VIEW (navigation_tree_view)), 
			    "changed", ((GCallback) gsb_gui_navigation_select_line), 
			    navigation_model );

    /* Create column */
    column = gtk_tree_view_column_new ( );

    /* Pixbuf renderer. */
    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, FALSE);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "pixbuf", NAVIGATION_PIX);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "visible", NAVIGATION_PIX_VISIBLE);
    gtk_tree_view_column_set_expand ( column, FALSE );

    /* Text renderer. */
    renderer = gtk_cell_renderer_text_new();
    renderer -> xpad = 6;
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, TRUE);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "text", NAVIGATION_TEXT);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "weight", NAVIGATION_FONT);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "sensitive", NAVIGATION_SENSITIVE);

    gtk_tree_view_append_column ( GTK_TREE_VIEW ( navigation_tree_view ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );
    /* Account list */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "resume.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &account_iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &account_iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Accounts"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_HOME_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_REPORT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1);
    create_account_list ( GTK_TREE_MODEL(navigation_model) );

    /* Scheduler */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "scheduler.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Scheduler"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_SCHEDULER_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_REPORT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );

    /* Payees */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "payees.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Payees"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_PAYEES_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_REPORT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );

    /* Categories */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "categories.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Categories"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_CATEGORIES_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_REPORT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );

    /* Budgetary lines */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "budgetary_lines.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Budgetary lines"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_BUDGETARY_LINES_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_REPORT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );

    /* Reports */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "reports.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &reports_iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &reports_iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Reports"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_REPORTS_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_REPORT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );
    create_report_list ( GTK_TREE_MODEL(navigation_model), &reports_iter );

    /** FIXME (later) : define an api so that plugin register here itself.  */
    if ( gsb_plugin_find ( "g2banking" ) )
    {
	/* Gbanking */
	pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
							 "aqbanking.png", NULL ), NULL );
	gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
	gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
			   NAVIGATION_PIX, pixbuf,
			   NAVIGATION_TEXT, _("Aqbanking"), 
			   NAVIGATION_PIX_VISIBLE, TRUE, 
			   NAVIGATION_FONT, 800,
			   NAVIGATION_PAGE, GSB_AQBANKING_PAGE,
			   NAVIGATION_ACCOUNT, -1,
			   NAVIGATION_REPORT, -1,
			   NAVIGATION_SENSITIVE, 1,
			   -1 );
    }

    /* Finish tree. */
    gtk_tree_view_expand_all ( GTK_TREE_VIEW(navigation_tree_view) );
    gtk_box_pack_start ( GTK_BOX(vbox), sw, TRUE, TRUE, 0 );

    /* Create calendar (hidden for now). */
    scheduler_calendar = creation_partie_gauche_echeancier();
    gtk_box_pack_end ( GTK_BOX(vbox), scheduler_calendar, FALSE, FALSE, 0 );

    /* Create reconcile stuff (hidden for now). */
    reconcile_panel = creation_fenetre_equilibrage();
    gtk_box_pack_end ( GTK_BOX(vbox), reconcile_panel, FALSE, FALSE, 0 );

    gtk_widget_show_all ( vbox );
    gtk_widget_hide_all ( scheduler_calendar );
    gtk_widget_hide_all ( reconcile_panel );

    return vbox;
}



/**
 * return the current page selected
 * the value returned is defined by GSB_GENERAL_NOTEBOOK_PAGES
 *
 * \param
 *
 * \return a gint wich is the numero of the page
 *
 * */
gint gsb_gui_navigation_get_current_page ( void )
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gint page;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (navigation_tree_view));

    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
	return GSB_HOME_PAGE;

    gtk_tree_model_get ( GTK_TREE_MODEL (navigation_model),
			 &iter,
			 NAVIGATION_PAGE, &page,
			 -1);
    return page;
}



/**
 * return the account number selected
 *
 * \param
 *
 * \return a gint, the account number or -1 if none selected
 * */
gint gsb_gui_navigation_get_current_account ( void )
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gint page;

    if ( !navigation_tree_view )
	return -1;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (navigation_tree_view));

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return -1;

    gtk_tree_model_get ( GTK_TREE_MODEL (navigation_model),
			 &iter,
			 NAVIGATION_PAGE, &page,
			 -1);
    
    if ( page == GSB_ACCOUNT_PAGE )
    {
	gint account_number;
	gtk_tree_model_get (GTK_TREE_MODEL(navigation_model), &iter, NAVIGATION_ACCOUNT, &account_number, -1);

	return account_number;
    }
    return -1;
}



/**
 * Return the number of the current selected report
 *
 * \param
 *
 * \return the current number of the report, or 0 if none selected
 * */
gint gsb_gui_navigation_get_current_report ( void )
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gint page;

    if ( ! navigation_tree_view )
    {
	return 0;
    }

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (navigation_tree_view));

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return 0;

    gtk_tree_model_get ( GTK_TREE_MODEL (navigation_model),
			 &iter,
			 NAVIGATION_PAGE, &page,
			 -1);
    
    if ( page == GSB_REPORTS_PAGE)
    {
	gint report_number;
	gtk_tree_model_get (GTK_TREE_MODEL(navigation_model), &iter, NAVIGATION_REPORT, &report_number, -1);

	return report_number;
    }
    return 0;
}




/**
 * Create a list of tree items that are shortcuts to accounts.
 *
 * \param model		Tree model to insert items into.
 * \param account_iter	Parent iter.
 */
void create_account_list ( GtkTreeModel * model )
{
    GSList *list_tmp;
    GtkTreeIter parent, child;
    GtkTreePath * path;

    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL(navigation_model), &parent );

    /* Remove childs if any. */
    while ( gtk_tree_model_iter_children ( model, &child, &parent ) )
    {
	gtk_tree_store_remove ( GTK_TREE_STORE(model), &child );
    }

    /* Fill in with accounts. */
    list_tmp = gsb_data_account_get_list_accounts ();
    while ( list_tmp )
    {
	gint i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( etat.show_closed_accounts || 
	     ! gsb_data_account_get_closed_account ( i ) )
	{
	    gsb_gui_navigation_add_account ( i );
	}

	list_tmp = list_tmp -> next;
    }

    /* Expand stuff */
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL(navigation_model), &parent );
    gtk_tree_view_expand_to_path ( GTK_TREE_VIEW(navigation_tree_view), path );
    gtk_tree_path_free ( path );
}



/**
 * Create a list of tree items that are shortcuts to reports.
 *
 * \param model		Tree model to insert items into.
 * \param reports_iter	Parent iter.
 */
void create_report_list ( GtkTreeModel * model, GtkTreeIter * reports_iter )
{
    GSList *tmp_list;
    GtkTreeIter iter;

    /* Fill in with reports */
    
    tmp_list = gsb_data_report_get_report_list ();

    while ( tmp_list )
    {
	gint report_number;

	report_number = gsb_data_report_get_report_number (tmp_list -> data);

	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, reports_iter);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
			   NAVIGATION_PIX_VISIBLE, FALSE, 
			   NAVIGATION_TEXT, gsb_data_report_get_report_name (report_number),
			   NAVIGATION_FONT, 400,
			   NAVIGATION_PAGE, GSB_REPORTS_PAGE,
			   NAVIGATION_ACCOUNT, -1,
			   NAVIGATION_SENSITIVE, 1,
			   NAVIGATION_REPORT, report_number,
			   -1 );
	
	tmp_list = tmp_list -> next;
    }
}


/**
 *
 *
 */
gboolean gsb_gui_navigation_line_visible_p ( GtkTreeModel * model, GtkTreeIter * iter,
					     gpointer data )
{
    guint account_nb, page;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), iter,
			 NAVIGATION_PAGE, &page,
			 NAVIGATION_ACCOUNT, &account_nb, 
			 -1 );

    if ( page != GSB_ACCOUNT_PAGE || account_nb < 0 )
	return TRUE;

    if ( (! gsb_data_account_get_closed_account(account_nb)) || etat.show_closed_accounts )
    {
        return TRUE;
    }

    return FALSE;
}



/**
 *
 *
 *
 */
gboolean navigation_sort_column ( GtkTreeModel * model, 
				  GtkTreeIter * a, GtkTreeIter * b, 
				  gpointer user_data )
{
    gint page_a, page_b, account_a, account_b, report_a, report_b;
    GSList * sort_reports = gsb_data_report_get_report_list ();
    gpointer preport_a, preport_b;

    if ( ! model )
	return FALSE;

    gtk_tree_model_get ( model, a, 
			 NAVIGATION_PAGE, &page_a,
			 NAVIGATION_ACCOUNT, &account_a,
			 NAVIGATION_REPORT, &report_a,
			 -1 );

    gtk_tree_model_get ( model, b, 
			 NAVIGATION_PAGE, &page_b,
			 NAVIGATION_ACCOUNT, &account_b,
			 NAVIGATION_REPORT, &report_b,
			 -1 );

    preport_a = gsb_data_report_get_report_by_no ( report_a );
    preport_b = gsb_data_report_get_report_by_no ( report_b );

    if ( page_a == GSB_ACCOUNT_PAGE && page_b == GSB_ACCOUNT_PAGE )
    {
	if ( g_slist_index ( sort_accounts, GINT_TO_POINTER (account_a)) >
	     g_slist_index ( sort_accounts, GINT_TO_POINTER (account_b)))
	{
	    return 1;
	}	

	return -1;
    }
    else if ( page_a == GSB_REPORTS_PAGE && page_b == GSB_REPORTS_PAGE )
    {
	if ( g_slist_index ( sort_reports, preport_a ) > 
	     g_slist_index ( sort_reports, preport_b ) )
	{
	    return 1;
	}

	return -1;
    }
    else
    {
	if ( page_a == page_b )
	{
	    return 0;
	}
	if ( page_a < page_b )
	{
	    return -1;
	}
	return 1;
    }
}



/**
 * Selects the account or management page with an optional account
 * selected in general notebook.
 *
 * \param selection	Selection of tree that triggered event.
 * \param selection	Model of tree that triggered event.
 *
 * \return		FALSE
 */
gboolean gsb_gui_navigation_update_notebook ( GtkTreeSelection * selection,
					      GtkTreeModel * model )
{
    GtkTreeIter iter;
    gint report_number;
    gint account_nb, page;

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return(FALSE);

    /* we go on the account page */ 

    gtk_tree_model_get (model, &iter, 
			NAVIGATION_PAGE, &page,
			NAVIGATION_ACCOUNT, &account_nb,
			NAVIGATION_REPORT, &report_number,
			-1 );
    gsb_gui_notebook_change_page ( (GsbGeneralNotebookPages) page );

    if ( page == GSB_ACCOUNT_PAGE && account_nb >= 0 )
    {
	gsb_data_account_list_gui_change_current_account ( GINT_TO_POINTER(account_nb) );
	remplissage_details_compte ();
    }

    if ( page == GSB_REPORTS_PAGE )
    {
	if ( report_number > 0 )
	{
	    gsb_gui_update_gui_to_report ( report_number );
	}
	else
	{
	    gsb_gui_unsensitive_report_widgets ();
	}
    }

    return FALSE;
}



/**
 * Iterator that iterates over the navigation pane model and update
 * iter of account that is equal to `data'.
 *
 * \param tree_model	Pointer to the model of the navigation tree.
 * \param path		Not used.
 * \param iter		Current iter to test.
 * \param data		Number of an account to match against the
 *			NAVIGATION_ACCOUNT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_update_account_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data )
{
    gint account_nb;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_ACCOUNT, &account_nb, 
			 -1 );

    if ( account_nb == GPOINTER_TO_INT ( data ) )
    {
	gsb_gui_navigation_update_account_iter ( tree_model, iter, GPOINTER_TO_INT(data) );
	return TRUE;
    }

    return FALSE;
}



/**
 * Iterator that iterates over the navigation pane model and update
 * iter of report that is equal to `data'.
 *
 * \param tree_model	Pointer to the model of the navigation tree.
 * \param path		Not used.
 * \param iter		Current iter to test.
 * \param data		Number of an report to match against the
 *			NAVIGATION_REPORT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_update_report_iterator ( GtkTreeModel * tree_model, 
							    GtkTreePath *path, 
							    GtkTreeIter *iter, 
							    gpointer data )
{
    gint report_nb;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_REPORT, &report_nb, 
			 -1 );

    if ( report_nb == GPOINTER_TO_INT ( data ) )
    {
	gsb_gui_navigation_update_report_iter ( tree_model, iter, GPOINTER_TO_INT ( data ));
	return TRUE;
    }

    return FALSE;
}



/**
 * Update informations for an report in navigation pane.
 *
 * \param report_nb	Number of the report that has to be updated.
 */
void gsb_gui_navigation_update_report ( gint report_number ) 
{
    gtk_tree_model_foreach ( GTK_TREE_MODEL(navigation_model),
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_update_report_iterator,
			     GINT_TO_POINTER ( report_number ) );
}



/**
 * Update contents of an iter with report data.
 *
 * \param model		Pointer to the model of the navigation tree.
 * \param report_iter	Iter to update.
 * \param data		Number of report as a reference.
 */
void gsb_gui_navigation_update_report_iter ( GtkTreeModel * model, 
					     GtkTreeIter * report_iter,
					     gint report_number )
{
    gtk_tree_store_set(GTK_TREE_STORE(model), report_iter, 
		       NAVIGATION_TEXT, gsb_data_report_get_report_name (report_number), 
		       NAVIGATION_PAGE, GSB_REPORTS_PAGE,
		       NAVIGATION_REPORT, report_number,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );
}



/**
 * Iterator that iterates over the navigation pane model and remove
 * iter of report that is equal to `data'.
 *
 * \param tree_model	Pointer to the model of the navigation tree.
 * \param path		Not used.
 * \param iter		Current iter to test.
 * \param data		Number of an report to match against the
 *			NAVIGATION_REPORT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_remove_report_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data )
{
    gint report;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_REPORT, &report, 
			 -1 );

    if ( report == GPOINTER_TO_INT (data))
    {
	gtk_tree_store_remove ( GTK_TREE_STORE(tree_model), iter );
	return TRUE;
    }

    return FALSE;
}



/**
 * Add an report to the navigation pane.
 *
 * \param report_nb	Report ID to add.
 */
void gsb_gui_navigation_add_report ( gint report_number )
{
    GtkTreeIter parent, iter;
    GtkTreeSelection * selection;
    GtkTreePath * path;

    path = gtk_tree_path_new ();
    gtk_tree_path_prepend_index ( path, GSB_REPORTS_PAGE - 1 );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL(navigation_model), &parent, path );
    gtk_tree_store_append ( GTK_TREE_STORE(navigation_model), &iter, &parent );
    gtk_tree_view_expand_to_path ( GTK_TREE_VIEW(navigation_tree_view), path );

    gsb_gui_navigation_update_report_iter ( GTK_TREE_MODEL(navigation_model), &iter, 
					    report_number );    

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    gtk_tree_selection_select_iter ( selection, &iter );
}



/**
 * Remove report from the navigation pane.
 *
 * \param report_nb	Report ID to add.
 */
void gsb_gui_navigation_remove_report ( gint report_number )
{
    gtk_tree_model_foreach ( GTK_TREE_MODEL(navigation_model), 
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_remove_report_iterator, 
			     GINT_TO_POINTER ( report_number ) );
   
}



/**
 * Update informations for an account in navigation pane.
 *
 * \param account_nb	Number of the account that has to be updated.
 */
void gsb_gui_navigation_update_account ( gint account_nb )
{
    gtk_tree_model_foreach ( GTK_TREE_MODEL(navigation_model), 
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_update_account_iterator, 
			     GINT_TO_POINTER ( account_nb ) );
}



/**
 * Update contents of an iter with account data.
 *
 * \param model		Pointer to the model of the navigation tree.
 * \param account_iter	Iter to update.
 * \param data		Number of account as a reference.
 */
void gsb_gui_navigation_update_account_iter ( GtkTreeModel * model, 
					      GtkTreeIter * account_iter,
					      gint account_nb )
{
    GdkPixbuf * pixbuf;
    gchar * account_icon;
	    
    switch ( gsb_data_account_get_kind ( account_nb ) )
    {
	case GSB_TYPE_BANK:
	    account_icon = "bank-account";
	    break;

	case GSB_TYPE_CASH:
	    account_icon = "money";
	    break;

	case GSB_TYPE_ASSET:
	case GSB_TYPE_LIABILITIES:
	    account_icon = "asset";
	    break;

	default:
	    account_icon = "warnings";
	    break;
    }

    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, 
						     C_DIRECTORY_SEPARATOR,
						     account_icon, ".png", NULL ), NULL );
    gtk_tree_store_set(GTK_TREE_STORE(model), account_iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_TEXT, gsb_data_account_get_name ( account_nb ), 
		       NAVIGATION_FONT, 400,
		       NAVIGATION_PAGE, GSB_ACCOUNT_PAGE,
		       NAVIGATION_ACCOUNT, account_nb,
		       NAVIGATION_SENSITIVE, !gsb_data_account_get_closed_account ( account_nb ),
		       -1 );
}



/**
 * Iterator that iterates over the navigation pane model and remove
 * iter of account that is equal to `data'.
 *
 * \param tree_model	Pointer to the model of the navigation tree.
 * \param path		Not used.
 * \param iter		Current iter to test.
 * \param data		Number of an account to match against the
 *			NAVIGATION_ACCOUNT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_remove_account_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data )
{
    gint account_nb;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_ACCOUNT, &account_nb, 
			 -1 );

    if ( account_nb == GPOINTER_TO_INT ( data ) )
    {
	gtk_tree_store_remove ( GTK_TREE_STORE(tree_model), iter );
	return TRUE;
    }

    return FALSE;
}



/**
 * Add an account to the navigation pane.
 *
 * \param account_nb	Account ID to add.
 */
void gsb_gui_navigation_add_account ( gint account_nb )
{
    GtkTreeIter parent, iter;
    GtkTreeSelection * selection;

    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL(navigation_model), &parent );
    gtk_tree_store_append ( GTK_TREE_STORE(navigation_model), &iter, &parent );

    gsb_gui_navigation_update_account_iter ( GTK_TREE_MODEL(navigation_model), &iter, account_nb );    

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    gtk_tree_selection_select_iter ( selection, &iter );
}



/**
 * Remove account from the navigation pane.
 *
 * \param account_nb	Account ID to remove.
 */
void gsb_gui_navigation_remove_account ( gint account_nb )
{
    gtk_tree_model_foreach ( GTK_TREE_MODEL(navigation_model), 
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_remove_account_iterator, 
			     GINT_TO_POINTER ( account_nb ) );
   
}



/**
 * Callback executed when the selection of the navigation tree
 * changed.
 *
 * \param selection	The selection that triggered event.
 * \param model		Tree model associated to selection.
 *
 * \return FALSE
 */
gboolean gsb_gui_navigation_select_line ( GtkTreeSelection * selection,
					  GtkTreeModel * model )
{
    GtkTreeIter iter;
    gchar * title, * suffix = "";
    gint account_nb, page;
    gint report_number;
    gint currency_number;

    devel_debug ("gsb_gui_navigation_select_line");

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return FALSE;

    gtk_tree_model_get (model, &iter, NAVIGATION_PAGE, &page, -1);

    gsb_gui_navigation_update_notebook ( selection, model );
    if ( page == GSB_ACCOUNT_PAGE )
	gsb_menu_update_accounts_in_menus ();


    switch ( page )
    {
	case GSB_HOME_PAGE:
	    if ( titre_fichier && strlen ( titre_fichier ) )
	    {
		title = g_strconcat ( "Grisbi : " , titre_fichier, NULL );
	    }
	    else
	    {
		title = g_strconcat ( "Grisbi : " , _("My accounts"), NULL );
	    }
	    break;

	case GSB_ACCOUNT_PAGE:
	    gtk_tree_model_get (model, &iter, NAVIGATION_ACCOUNT, &account_nb, -1);
	    title = g_strconcat ( _("Account transactions"), " : ",
				  gsb_data_account_get_name ( account_nb ),
				  NULL );
	    if ( gsb_data_account_get_closed_account ( account_nb ) )
	    {
		title = g_strconcat ( title, " (", _("closed"), ")", NULL );
	    }

	    currency_number = gsb_data_account_get_currency ( compte_courant_onglet );
	    suffix = g_strdup_printf ( "%s %s", 
				       gsb_real_get_string (gsb_data_account_get_current_balance (compte_courant_onglet)),
				       gsb_data_currency_get_code (currency_number));
	    gsb_menu_update_view_menu ( account_nb );
	    break;

	case GSB_PAYEES_PAGE:
	    title = _("Payees");
	    break;

	case GSB_CATEGORIES_PAGE:
	    title = _("Categories");
	    break;

	case GSB_BUDGETARY_LINES_PAGE:
	    title = _("Budgetary lines");
	    break;

	case GSB_AQBANKING_PAGE:
	    /** FIXME (later) : define an api so that plugin register here itself.  */
	    if ( gsb_plugin_find ( "g2banking" ) )
	    {
		title = _("AqBanking");
	    }
	    break;

	case GSB_SCHEDULER_PAGE:
	    title = _("Scheduled transactions");
	    break;

	case GSB_REPORTS_PAGE:
	    gtk_tree_model_get (model, &iter, NAVIGATION_REPORT, &report_number, -1);
	    if ( report_number )
	    {
		title = g_strconcat ( _("Report"), " : ", gsb_data_report_get_report_name (report_number), NULL );
	    }
	    else
	    {
		title = _("Reports");
	    }
	    break;

	default:
	    title = "B0rk";
	    break;
    }

    gsb_gui_headings_update ( title, suffix );
    return FALSE;
}



/**
 * Set the selection of the navigation list depending on desired
 * page and/or account or report.
 *
 * \param page		Page to switch to.
 * \param account_nb	If page is GSB_ACCOUNT_PAGE, switch to given
 *			account.
 * \param report	If page is GSB_REPORTS, switch to given
 *			report.
 * 
 * \return		TRUE on success.
 */
gboolean gsb_gui_navigation_set_selection ( gint page, gint account_nb, gpointer report )
{
    GtkTreeIter iter;
    GtkTreeSelection * selection;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    g_return_val_if_fail ( selection, FALSE );

    /* This is no blocked because we want things like headings to be updated. [benj]  */
/*     g_signal_handlers_block_by_func ( selection, */
/* 				      gsb_gui_navigation_select_line, */
/* 				      navigation_model ); */

    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL(navigation_model), &iter );

    gsb_gui_navigation_set_selection_branch ( selection, &iter, page, account_nb, 
					      report );

    return TRUE;
}



/**
 * Set the selection of the navigation list depending on desired page
 * and/or account or report, but only for a branch of the tree.
 *
 * \param selection	Selection to set.
 * \param iter		Current iter to iterate over.
 * \param page		Page to switch to.
 * \param account_nb	If page is GSB_ACCOUNT_PAGE, switch to given
 *			account.
 * \param report	If page is GSB_REPORTS, switch to given
 *			report.
 * 
 * \return		TRUE on success.
 */
void gsb_gui_navigation_set_selection_branch ( GtkTreeSelection * selection, 
					       GtkTreeIter * iter, gint page, 
					       gint account_nb, gpointer report )
{
    if ( gtk_tree_model_iter_has_child ( GTK_TREE_MODEL(navigation_model), iter ) )
    {
	GtkTreeIter child;
    
	gtk_tree_model_iter_children ( GTK_TREE_MODEL(navigation_model), &child, iter );
	gsb_gui_navigation_set_selection_branch ( selection, &child, 
						  page, account_nb, report );
    }

    do 
    {
	gint iter_page, iter_account_nb;
	gpointer iter_report;

	gtk_tree_model_get ( GTK_TREE_MODEL(navigation_model), iter, 
			     NAVIGATION_REPORT, &iter_report, 
			     NAVIGATION_ACCOUNT, &iter_account_nb,
			     NAVIGATION_PAGE, &iter_page,
			     -1 );

	if ( iter_page == page &&
	     ! ( page == GSB_ACCOUNT_PAGE && 
		 iter_account_nb != account_nb ) &&
	     ! ( page == GSB_REPORTS_PAGE &&
		 iter_report != report ) )
	{
	    gtk_tree_selection_select_iter ( selection, iter );
	}
    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL(navigation_model), iter ) );

/*     g_signal_handlers_unblock_by_func ( selection, */
/* 					gsb_gui_navigation_select_line, */
/* 					navigation_model ); */

    return;
}



/**
 *
 *
 *
 */
gboolean gsb_gui_navigation_select_prev ()
{
    GtkTreeSelection * selection;
    GtkTreePath * path;
    GtkTreeModel * model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    g_return_val_if_fail ( selection, FALSE );
    
    gtk_tree_selection_get_selected ( selection, &model, &iter );
    path = gtk_tree_model_get_path ( model, &iter );
    g_return_val_if_fail ( path, TRUE );

    if ( ! gtk_tree_path_prev ( path ) )
    {
	gtk_tree_path_up ( path );
    }
    else
    {
	gtk_tree_model_get_iter ( model, &iter, path );
	if ( gtk_tree_model_iter_has_child ( model, &iter ) )
	{
	    GtkTreeIter parent = iter;
	    gtk_tree_model_iter_nth_child ( model, &iter, &parent, 
					    gtk_tree_model_iter_n_children ( model, 
									     &parent ) - 1 );
	    path = gtk_tree_model_get_path ( model, &iter );
	}
    }
    
    gtk_tree_selection_select_path ( selection, path );

    return FALSE;
}



/**
 *
 *
 *
 */
gboolean gsb_gui_navigation_select_next ()
{
    GtkTreeSelection * selection;
    GtkTreePath * path;
    GtkTreeModel * model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    g_return_val_if_fail ( selection, FALSE );
    
    gtk_tree_selection_get_selected ( selection, &model, &iter );
    path = gtk_tree_model_get_path ( model, &iter );
    g_return_val_if_fail ( path, TRUE );

    if ( gtk_tree_model_iter_has_child ( model, &iter ) )
    {
	gtk_tree_path_down ( path );
    }
    else
    {
	if ( ! gtk_tree_model_iter_next ( model, &iter ) )
	{
	    gtk_tree_path_up ( path );
	    gtk_tree_path_next ( path );
	}
	else
	{ 
	    path = gtk_tree_model_get_path ( model, &iter );
	}
    }

    gtk_tree_selection_select_path ( selection, path );

    return FALSE;
}



/** 
 * Check the key pressed on the navigation tree view,
 * if need, stop the event to do another thing with that key
 *
 * \param tree_view the navigation tree_view
 * \param ev the key event pressed
 * \param model
 *
 * \return FALSE : the signal continue / TRUE : the signal is stopped here
 * */
gboolean gsb_gui_navigation_check_key_press ( GtkWidget *tree_view,
					      GdkEventKey *ev,
					      GtkTreeModel *model )
{
    gint page;
    GtkTreeIter iter;

   if (! gtk_tree_selection_get_selected ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
									NULL,
									&iter))
	return FALSE;

   gtk_tree_model_get (model, &iter, NAVIGATION_PAGE, &page, -1 );

    switch ( page )
    {
	case GSB_HOME_PAGE:
	    /* nothing to do here for now */
	    break;

	case GSB_ACCOUNT_PAGE:
	    switch ( ev -> keyval )
	    {
		case GDK_Return :		/* entrée */
		case GDK_KP_Enter :
		case GDK_Up :		/* touches flèche haut */
		case GDK_KP_Up :
		case GDK_Down :		/* touches flèche bas */
		case GDK_KP_Down :
		case GDK_Delete:		/*  del  */
		case GDK_P:			/* touche P */
		case GDK_p:			/* touche p */
		case GDK_r:			/* touche r */
		case GDK_R:			/* touche R */
		    gsb_transactions_list_key_press ( gsb_transactions_list_get_tree_view (),
						      ev );
		    return TRUE;
	    }
	    break;

	case GSB_PAYEES_PAGE:
	    /* nothing to do here for now */
	    break;

	case GSB_CATEGORIES_PAGE:
	    /* nothing to do here for now */
	    break;

	case GSB_BUDGETARY_LINES_PAGE:
	    /* nothing to do here for now */
	    break;

	case GSB_SCHEDULER_PAGE:
	    switch ( ev -> keyval )
	    {
		case GDK_Return :		/* touches entrée */
		case GDK_KP_Enter :
		case GDK_Up :		/* touches flèche haut */
		case GDK_KP_Up :
		case GDK_Down :		/* touches flèche bas */
		case GDK_KP_Down :
		case GDK_Delete :               /*  del  */

		    gsb_scheduler_list_key_press  ( gsb_scheduler_list_get_tree_view (),
						    ev );
		    return TRUE;
	    }
	    break;

	case GSB_REPORTS_PAGE:
	    /* nothing to do here for now */
	    break;
    }

    return FALSE;
}



/**
 * Fill the drag & drop structure with the path of selected column.
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_source		Not used.
 * \param path			Original path for the gtk selection.
 * \param selection_data	A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 */
gboolean navigation_tree_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
					 GtkSelectionData * selection_data )
{
    if ( path )
    {
	gtk_tree_set_row_drag_data ( selection_data, GTK_TREE_MODEL(navigation_model), 
				     path );
    }

    return FALSE;
}



/**
 *  
 *
 */
gboolean navigation_drag_data_received ( GtkTreeDragDest * drag_dest,
					 GtkTreePath * dest_path,
					 GtkSelectionData * selection_data )
{
    if ( dest_path && selection_data )
    {
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreePath * orig_path;
	gchar * name;

	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);

	if ( gtk_tree_model_get_iter ( GTK_TREE_MODEL(model), &iter, dest_path ) ) 
	{
	    gint src_report, dst_report = -1;
	    gint src_account, dst_account = -1;

	    gtk_tree_model_get (GTK_TREE_MODEL(model) , &iter, 
				NAVIGATION_TEXT, &name, 
				NAVIGATION_REPORT, &dst_report, 
				NAVIGATION_ACCOUNT, &dst_account,
				-1 );
	
	    if ( gtk_tree_model_get_iter ( GTK_TREE_MODEL(model), &iter, orig_path ) )
	    {
		gtk_tree_model_get ( model, &iter, 
				     NAVIGATION_REPORT, &src_report, 
				     NAVIGATION_ACCOUNT, &src_account,
				     -1 );
	    }
	
	    navigation_change_account_order ( model, src_account, dst_account );
	    navigation_change_report_order ( model, src_report, dst_report );
	}
    }

    return FALSE;
}



/**
 *
 *
 */
gboolean navigation_row_drop_possible ( GtkTreeDragDest * drag_dest, 
					GtkTreePath * dest_path,
					GtkSelectionData * selection_data )
{
    if ( dest_path && selection_data )
    {
	GtkTreePath * orig_path;
	GtkTreeModel * model;
	gint src_report, dst_report = -1;
	gint src_account, dst_account = -1, dst_page = -1;
	GtkTreeIter iter;

	gtk_tree_get_row_drag_data ( selection_data, &model, &orig_path );

	if ( gtk_tree_model_get_iter ( model, &iter, orig_path ) )
	{
	    gtk_tree_model_get ( model, &iter, 
				 NAVIGATION_REPORT, &src_report, 
				 NAVIGATION_ACCOUNT, &src_account,
				 -1 );
	}
	if ( gtk_tree_model_get_iter ( model, &iter, dest_path ) )
	{
	    gtk_tree_model_get ( model, &iter, 
				 NAVIGATION_REPORT, &dst_report, 
				 NAVIGATION_ACCOUNT, &dst_account,
				 NAVIGATION_PAGE, &dst_page,
				 -1 );
	}
	
	/* We handle an account */
	if ( src_account >= 0 && dst_account >= 0 )
	{
	    printf ("> Possible (account)\n");
	    return TRUE;
	}
	/* We handle a report */
	else if ( src_report > 0 && dst_report > 0 )
	{
	    printf ("> Possible (report, %d, %d)\n", src_report, dst_report);
	    return TRUE;
	}
    }

    return FALSE;
}



/**
 *
 *
 *
 */
void navigation_change_account_order ( GtkTreeModel * model, gint orig, gint dest )
{
    GSList * tmp, * dest_pointer;

    /* No customization. */
    if ( ! g_slist_length ( sort_accounts ) )
    {
	tmp = gsb_data_account_get_list_accounts ();
	while ( tmp )
	{
	    sort_accounts = g_slist_append ( sort_accounts, 
					     GINT_TO_POINTER (gsb_data_account_get_no_account ( tmp -> data )));
	    tmp = tmp -> next;
	}
    }

    sort_accounts = g_slist_remove ( sort_accounts, GINT_TO_POINTER (orig));
    dest_pointer = g_slist_find ( sort_accounts, (gpointer) dest );
    if ( dest_pointer )
    {
	sort_accounts = g_slist_insert ( sort_accounts,
					 GINT_TO_POINTER (orig),
					 g_slist_position ( sort_accounts, 
							    dest_pointer ) + 1 );
    }

    gsb_data_account_reorder ( sort_accounts );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(model),
					   NAVIGATION_PAGE, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(model), 
				      NAVIGATION_PAGE, navigation_sort_column,
				      NULL, NULL );
}



/**
 *
 *
 *
 */
void navigation_change_report_order ( GtkTreeModel * model, gint orig, gint dest )
{
    GSList * tmp, * dest_pointer;
    gpointer orig_report = gsb_data_report_get_report_by_no ( orig );
    gpointer dest_report = gsb_data_report_get_report_by_no ( dest );

    printf ("> %d, %d\n", orig, dest);

    tmp = gsb_data_report_get_report_list ();

    dest_pointer = g_slist_find ( tmp, dest_report );
    if ( dest_pointer )
    {
	tmp = g_slist_remove ( tmp, orig_report );
	tmp = g_slist_insert ( tmp, orig_report,
			       g_slist_position ( tmp, dest_pointer ) + 1 );
	gsb_data_report_set_report_list ( tmp );
    }

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(model),
					   NAVIGATION_PAGE, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(model), 
				      NAVIGATION_PAGE, navigation_sort_column,
				      NULL, NULL );
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
