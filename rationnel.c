/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux.
 *
 *   Copyright (C) 2015 Giuliana Bianchi, Adrien Boussicault, Thomas Place, Marc Zeitoun
 *
 *    This Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This Library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this Library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rationnel.h"
#include "ensemble.h"
#include "automate.h"
#include "parse.h"
#include "scan.h"
#include "outils.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
int Numb = 0;
int yyparse(Rationnel **rationnel, yyscan_t scanner);

Rationnel *rationnel(Noeud etiquette, char lettre, int position_min, int position_max, void *data, Rationnel *gauche, Rationnel *droit, Rationnel *pere)
{
    Rationnel *rat;
    rat = (Rationnel *) malloc(sizeof(Rationnel));

    rat->etiquette = etiquette;
    rat->lettre = lettre;
    rat->position_min = position_min;
    rat->position_max = position_max;
    rat->data = data;
    rat->gauche = gauche;
    rat->droit = droit;
    rat->pere = pere;
    return rat;
}

Rationnel *Epsilon()
{
    return rationnel(EPSILON, 0, 0, 0, NULL, NULL, NULL, NULL);
}

Rationnel *Lettre(char l)
{
    return rationnel(LETTRE, l, 0, 0, NULL, NULL, NULL, NULL);
}

Rationnel *Union(Rationnel* rat1, Rationnel* rat2)
{
    // Cas particulier où rat1 est vide
    if (!rat1)
        return rat2;

    // Cas particulier où rat2 est vide
    if (!rat2)
        return rat1;

    return rationnel(UNION, 0, 0, 0, NULL, rat1, rat2, NULL);
}

Rationnel *Concat(Rationnel* rat1, Rationnel* rat2)
{
    if (!rat1 || !rat2)
        return NULL;

    if (get_etiquette(rat1) == EPSILON)
        return rat2;

    if (get_etiquette(rat2) == EPSILON)
        return rat1;

    return rationnel(CONCAT, 0, 0, 0, NULL, rat1, rat2, NULL);
}

Rationnel *Star(Rationnel* rat)
{
    return rationnel(STAR, 0, 0, 0, NULL, rat, NULL, NULL);
}

bool est_racine(Rationnel* rat)
{
    return (rat->pere == NULL);
}

Noeud get_etiquette(Rationnel* rat)
{
    return rat->etiquette;
}

char get_lettre(Rationnel* rat)
{
    assert (get_etiquette(rat) == LETTRE);
    return rat->lettre;
}

int get_position_min(Rationnel* rat)
{
    assert (get_etiquette(rat) == LETTRE);
    return rat->position_min;
}

int get_position_max(Rationnel* rat)
{
    assert (get_etiquette(rat) == LETTRE);
    return rat->position_max;
}

void set_position_min(Rationnel* rat, int valeur)
{
    assert (get_etiquette(rat) == LETTRE);
    rat->position_min = valeur;
    return;
}

void set_position_max(Rationnel* rat, int valeur)
{
    assert (get_etiquette(rat) == LETTRE);
    rat->position_max = valeur;
    return;
}

Rationnel *fils_gauche(Rationnel* rat)
{
    assert((get_etiquette(rat) == CONCAT) || (get_etiquette(rat) == UNION));
    return rat->gauche;
}

Rationnel *fils_droit(Rationnel* rat)
{
    assert((get_etiquette(rat) == CONCAT) || (get_etiquette(rat) == UNION));
    return rat->droit;
}

Rationnel *fils(Rationnel* rat)
{
    assert(get_etiquette(rat) == STAR);
    return rat->gauche;
}

Rationnel *pere(Rationnel* rat)
{
    assert(!est_racine(rat));
    return rat->pere;
}

void print_rationnel(Rationnel* rat)
{
    if (rat == NULL)
    {
        printf("∅");
        return;
    }

    switch(get_etiquette(rat))
    {
        case EPSILON:
            printf("ε");         
            break;

        case LETTRE:
            printf("%c", get_lettre(rat));
            break;

        case UNION:
            printf("(");
            print_rationnel(fils_gauche(rat));
            printf(" + ");
            print_rationnel(fils_droit(rat));
            printf(")");         
            break;

        case CONCAT:
            printf("[");
            print_rationnel(fils_gauche(rat));
            printf(" . ");
            print_rationnel(fils_droit(rat));
            printf("]");         
            break;

        case STAR:
            printf("{");
            print_rationnel(fils(rat));
            printf("}*");         
            break;

        default:
            assert(false);
            break;
    }
}

Rationnel *expression_to_rationnel(const char *expr)
{
    Rationnel *rat;
    yyscan_t scanner;
    YY_BUFFER_STATE state;

    // Initialisation du scanner
    if (yylex_init(&scanner))
        return NULL;

    state = yy_scan_string(expr, scanner);

    // Test si parsing ok.
    if (yyparse(&rat, scanner)) 
        return NULL;

    // Libération mémoire
    yy_delete_buffer(state, scanner);

    yylex_destroy(scanner);

    return rat;
}

void rationnel_to_dot(Rationnel *rat, char* nom_fichier)
{
    FILE *fp = fopen(nom_fichier, "w+");
    rationnel_to_dot_aux(rat, fp, -1, 1);
}

int rationnel_to_dot_aux(Rationnel *rat, FILE *output, int pere, int noeud_courant)
{   
    int saved_pere = noeud_courant;

    if (pere >= 1)
        fprintf(output, "\tnode%d -> node%d;\n", pere, noeud_courant);
    else
        fprintf(output, "digraph G{\n");

    switch(get_etiquette(rat))
    {
        case LETTRE:
            fprintf(output, "\tnode%d [label = \"%c-%d\"];\n", noeud_courant, get_lettre(rat), rat->position_min);
            noeud_courant++;
            break;

        case EPSILON:
            fprintf(output, "\tnode%d [label = \"ε-%d\"];\n", noeud_courant, rat->position_min);
            noeud_courant++;
            break;

        case UNION:
            fprintf(output, "\tnode%d [label = \"+ (%d/%d)\"];\n", noeud_courant, rat->position_min, rat->position_max);
            noeud_courant = rationnel_to_dot_aux(fils_gauche(rat), output, noeud_courant, noeud_courant+1);
            noeud_courant = rationnel_to_dot_aux(fils_droit(rat), output, saved_pere, noeud_courant+1);
            break;

        case CONCAT:
            fprintf(output, "\tnode%d [label = \". (%d/%d)\"];\n", noeud_courant, rat->position_min, rat->position_max);
            noeud_courant = rationnel_to_dot_aux(fils_gauche(rat), output, noeud_courant, noeud_courant+1);
            noeud_courant = rationnel_to_dot_aux(fils_droit(rat), output, saved_pere, noeud_courant+1);
            break;

        case STAR:
            fprintf(output, "\tnode%d [label = \"* (%d/%d)\"];\n", noeud_courant, rat->position_min, rat->position_max);
            noeud_courant = rationnel_to_dot_aux(fils(rat), output, noeud_courant, noeud_courant+1);
            break;

        default:
            assert(false);
            break;
    }
    if (pere < 0)
        fprintf(output, "}\n");
    return noeud_courant;
}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  numeroter_rationnel_aux
 *  Description:  recursive function to number the letter of a regex 
 * =====================================================================================
 */
Rationnel* numeroter_rationnel_aux(Rationnel *rat){

    Rationnel *tm;
    if (rat == NULL)
    {
        return rat;
    }

    switch(get_etiquette(rat))
    {
        case EPSILON:
            return rat;

        case LETTRE:
            rat->position_min = Numb;
            rat->position_max = Numb;
            Numb++;
            return rat;

        case UNION:
            rat->position_min = numeroter_rationnel_aux(fils_gauche(rat))->position_min;
            rat->position_max = numeroter_rationnel_aux(fils_droit(rat))->position_max;

            return rat;

        case CONCAT:
            rat->position_min = numeroter_rationnel_aux(fils_gauche(rat))->position_min;
            rat->position_max = numeroter_rationnel_aux(fils_droit(rat))->position_max;

            return rat;

        case STAR:
            tm =  numeroter_rationnel_aux(fils(rat)); 
            rat->position_min =tm->position_min;
            rat->position_max = tm->position_max;

            return rat;

        default:
            assert(false);
            break;
    }


}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  numeroter_rationnel
 *  Description:  insert the respective numbers for each letter of the regex
 * =====================================================================================
 */
void numeroter_rationnel(Rationnel *rat)
{

    Numb = 1;
    Rationnel * temp = numeroter_rationnel_aux(rat);
    rat = temp;
}



bool contient_mot_vide(Rationnel *rat)
{

    if (rat == NULL)
    {
        return true;
    }

    switch(get_etiquette(rat))
    {
        case EPSILON:
            return true;

        case LETTRE:
            return false;

        case UNION:
            return contient_mot_vide(fils_gauche(rat)) || contient_mot_vide(fils_droit(rat));

        case CONCAT:
            return contient_mot_vide(fils_gauche(rat)) && contient_mot_vide(fils_droit(rat));

        case STAR:
            return true;

        default:
            assert(false);
            break;
    }

}




/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  premier
 *  Description:  list of the first possible letters of a regex
 * =====================================================================================
 */
Ensemble *premier(Rationnel *rat)
{
    Ensemble *tmp =  creer_ensemble( NULL, NULL, NULL );
    if (rat == NULL)
    {
        return tmp;
    }

    switch(get_etiquette(rat))
    {
        case EPSILON:
            return tmp;

        case LETTRE:
            ajouter_element(tmp,get_position_min(rat)); 
            return tmp;

        case UNION:
            return creer_union_ensemble(premier(fils_gauche(rat)),premier(fils_droit(rat)));

        case CONCAT:
            if(contient_mot_vide(fils_gauche(rat))){
                return creer_union_ensemble(premier(fils_gauche(rat)),premier(fils_droit(rat)));

            }else{
                return premier(fils_gauche(rat));

            }


        case STAR:
            return premier(fils(rat));

        default:
            assert(false);
            break;
    }


}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dernier
 *  Description:  return a list of the possible las letters of a regex
 * =====================================================================================
 */
Ensemble *dernier(Rationnel *rat)
{
    Ensemble *tmp =  creer_ensemble( NULL, NULL, NULL );
    if (rat == NULL)
    {
        return tmp;
    }

    switch(get_etiquette(rat))
    {
        case EPSILON:
            return tmp;

        case LETTRE:
            ajouter_element(tmp,get_position_min(rat)); 
            return tmp;

        case UNION:
            return creer_union_ensemble(dernier(fils_droit(rat)),dernier(fils_gauche(rat)));

        case CONCAT:
            if(contient_mot_vide(fils_droit(rat))){
                return creer_union_ensemble(dernier(fils_droit(rat)),dernier(fils_gauche(rat)));

            }else{
                return dernier(fils_droit(rat));

            }


        case STAR:
            return dernier(fils(rat));

        default:
            assert(false);
            break;
    }

}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  suivant
 *  Description:  return a list of the following letters of a given position
 * =====================================================================================
 */
Ensemble *suivant(Rationnel *rat, int position)
{
    Ensemble *tmp =  creer_ensemble( NULL, NULL, NULL );
    Ensemble *pr ;
    Ensemble *de ;

    if (rat == NULL)
    {
        return tmp;
    }

    switch(get_etiquette(rat))
    {
        case EPSILON:
            return tmp;

        case LETTRE:
            return tmp;

        case UNION:
            return creer_union_ensemble(suivant(fils_gauche(rat),position),suivant(fils_droit(rat),position));

        case CONCAT:
            pr = premier(fils_droit(rat));
            de = dernier(fils_gauche(rat));
            if(est_dans_l_ensemble(de,position)){            
                return creer_union_ensemble(creer_union_ensemble(suivant(fils_gauche(rat),position),suivant(fils_droit(rat),position)),pr) ;
            }
            return creer_union_ensemble(creer_union_ensemble(suivant(fils_gauche(rat),position),suivant(fils_droit(rat),position)),tmp) ;


        case STAR:
            pr = premier(fils(rat));
            de = dernier(fils(rat));
            if(est_dans_l_ensemble(de,position)){   

                return creer_union_ensemble(suivant(fils(rat),position),pr) ;
            }
            return creer_union_ensemble(suivant(fils(rat),position),tmp) ;


        default:
            assert(false);
            break;
    }


}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_lettre_by_position
 *  Description:  return the Rationnel of type lettre of a given position
 * =====================================================================================
 */
Rationnel *get_lettre_by_position(Rationnel *rat,int n){

    int gmin,gmax,rmin,rmax;

    if (rat == NULL)
    {
        return rat;
    }

    switch(get_etiquette(rat))
    {
        case EPSILON:
            return rat;

        case LETTRE:
            return rat;

        case UNION:
            gmin = fils_gauche(rat)->position_min;
            gmax = fils_gauche(rat)->position_max;
            rmin = fils_droit(rat)->position_min;
            rmax = fils_droit(rat)->position_max;
            if((gmin <= n) && (n <= gmax)){
                return get_lettre_by_position(fils_gauche(rat),n);
            }else if((rmin <= n) && (n <= rmax)){
                return get_lettre_by_position(fils_droit(rat),n);
            }
            return rat;


        case CONCAT:
            gmin = fils_gauche(rat)->position_min;
            gmax = fils_gauche(rat)->position_max;
            rmin = fils_droit(rat)->position_min;
            rmax = fils_droit(rat)->position_max;


            if((gmin <= n) && (n <= gmax)){
                return get_lettre_by_position(fils_gauche(rat),n);
            }else if((rmin <= n) && (n <= rmax)){
                return get_lettre_by_position(fils_droit(rat),n);
            }
            return rat;



        case STAR:         
            return get_lettre_by_position(fils(rat),n);

        default:
            assert(false);
            break;
    }



}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  Glushkov
 *  Description: transform a regex in atomata
 * =====================================================================================
 */
Automate *Glushkov(Rationnel *rat)
{
    Automate *temp = creer_automate();
    ajouter_etat_initial( temp, 0 );
    numeroter_rationnel(rat);
    const Ensemble *pre = premier(rat);
    const Ensemble *der = dernier(rat);
    int max = rat->position_max;




    /*-----------------------------------------------------------------------------
     *  iterates each one of the "premiers" and creat the transition from the state 0
     *-----------------------------------------------------------------------------*/
    Ensemble_iterateur it;
    for( it = premier_iterateur_ensemble( pre );
            ! iterateur_ensemble_est_vide( it );
            it = iterateur_suivant_ensemble( it )
       ){
        int nb = get_element(it);
        ajouter_transition( temp, 0, get_lettre(get_lettre_by_position(rat,nb)), nb );
    }



    /*-----------------------------------------------------------------------------
     *  iterates each state from 1 to and create the transitions 1-->suivant
     *-----------------------------------------------------------------------------*/
    int i;
    for(i = 1;i<=max;i++){
        Ensemble *suiv = suivant(rat,i);

        Ensemble_iterateur it2;
        for( it2 = premier_iterateur_ensemble( suiv );
                ! iterateur_ensemble_est_vide( it2 );
                it2 = iterateur_suivant_ensemble( it2 )
           ){
            int nb = get_element(it2);


            ajouter_transition( temp, i, get_lettre(get_lettre_by_position(rat,nb)), nb );


        }

    }



    /*-----------------------------------------------------------------------------
     *  create the final states based on the list of "derniers"
     *-----------------------------------------------------------------------------*/
    Ensemble_iterateur it3;
    for( it3 = premier_iterateur_ensemble( der );
            ! iterateur_ensemble_est_vide( it3 );
            it3 = iterateur_suivant_ensemble( it3 )
       ){
        int nb = get_element(it3);
        ajouter_etat_final( temp, nb );

    }


    /*-----------------------------------------------------------------------------
     *  if the regex accept Epsilon add the state 0 as final
     *-----------------------------------------------------------------------------*/
    if(contient_mot_vide(rat)){
        ajouter_etat_final( temp, 0 );


    }

    return temp;



}







/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  meme_langage
 *  Description:  compare 2 regex and return true if both are correspondent to the same language
 * =====================================================================================
 */

bool meme_langage (const char *expr1, const char* expr2)
{



    /*-----------------------------------------------------------------------------
     *  create the Rationnel 
     *-----------------------------------------------------------------------------*/
    Rationnel *rat1 = expression_to_rationnel(expr1);
    Rationnel *rat2 = expression_to_rationnel(expr2);
    
    /*-----------------------------------------------------------------------------
     *  create the automata minimal
     *-----------------------------------------------------------------------------*/
    const Automate  *aut1 = creer_automate_minimal(Glushkov(rat1));
    const Automate  *aut2 = creer_automate_minimal(Glushkov(rat2));
   
    
    /*-----------------------------------------------------------------------------
     *  check if the list of states,initials,finals are equals and if the alphabet is equal
     *-----------------------------------------------------------------------------*/
    bool test = iterateur_ensemble_est_vide(premier_iterateur_ensemble(creer_difference_ensemble(get_etats(aut1),get_etats(aut2)))) 
        && iterateur_ensemble_est_vide(premier_iterateur_ensemble(creer_difference_ensemble(get_initiaux(aut1),get_initiaux(aut2))))
        && iterateur_ensemble_est_vide(premier_iterateur_ensemble(creer_difference_ensemble(get_finaux(aut1),get_finaux(aut2))))
        && iterateur_ensemble_est_vide(premier_iterateur_ensemble(creer_difference_ensemble(get_alphabet(aut1),get_alphabet(aut2))));


    
    /*-----------------------------------------------------------------------------
     *  now if the previus tests passed we check if the transitions are equal
     *-----------------------------------------------------------------------------*/
    if(test){
        Table_iterateur tit1,tit2;
        Ensemble_iterateur eit1,eit2;
        for(
                tit1 = premier_iterateur_table( aut1->transitions ),
                tit2 =  premier_iterateur_table( aut2->transitions );
                ! iterateur_est_vide( tit1 ) &&
                ! iterateur_est_vide( tit2 );
                tit1 = iterateur_suivant_table( tit1 ),
                tit2 = iterateur_suivant_table( tit2 )

           ){
            Cle * cle1 = (Cle*) get_cle( tit1 );
            Ensemble * fins1 = (Ensemble*) get_valeur( tit1 );
            Cle * cle2 = (Cle*) get_cle( tit2 );
            Ensemble * fins2 = (Ensemble*) get_valeur( tit2 );
            for(
                    eit1 = premier_iterateur_ensemble( fins1 ),
                    eit2 = premier_iterateur_ensemble( fins2 );
                    ! iterateur_ensemble_est_vide( eit1 ) &&
                    ! iterateur_ensemble_est_vide( eit2 );
                    eit1 = iterateur_suivant_ensemble( eit1 ),
                    eit2 = iterateur_suivant_ensemble( eit2 )
               ){
                int fin1 = get_element( eit1 );
                int fin2 = get_element( eit2 );
                test = ((fin1 == fin2) && (cle1->origine == cle2->origine) && (cle1->lettre == cle2->lettre)); 
            }

        }

    }

    return test;





}







void print_ligne(Rationnel **ligne, int n)
{
    for (int j = 0; j <=n; j++)
    {
        print_rationnel(ligne[j]);
        if (j<n)
            printf("X%d\t+\t", j);
    }
    printf("\n");
}

void print_systeme(Systeme systeme, int n)
{
    for (int i = 0; i <= n-1; i++)
    {
        printf("X%d\t= ", i);
        print_ligne(systeme[i], n);
    }
}






/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  systeme_action
 *  Description:  populate the systeme
 * =====================================================================================
 */
void systeme_action(int origne,char lettre, int fin, void* data){
    Systeme sy = (Systeme)data;
    if(sy[origne][fin] == NULL){
        sy[origne][fin]=Lettre(lettre);
    }else{
        sy[origne][fin]=Union(sy[origne][fin],Lettre(lettre));

    }
}




/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  systeme
 *  Description:  creates the systeme for a given automata
 * =====================================================================================
 */
Systeme systeme(Automate *automate)
{
    int i = get_max_etat(automate)+1;
    Systeme sys;
    sys = malloc(sizeof(Rationnel)*i);
    int l,t;
    for(l=0;l<i;l++){
        sys[l]=malloc(sizeof(Rationnel)*(i+1));
    }
    for(l=0;l<i;l++){
        for(t=0;t<=i;t++){
            sys[l][t]=NULL;
        }
    }
    pour_toute_transition(automate,systeme_action,sys);

    /*-----------------------------------------------------------------------------
     *  insert Epsilon for the final states
     *-----------------------------------------------------------------------------*/
    Ensemble_iterateur it;
    const Ensemble * fins = get_finaux(automate);
    for( it = premier_iterateur_ensemble( fins );
            ! iterateur_ensemble_est_vide( it );
            it = iterateur_suivant_ensemble( it )
       ){
        int fin = get_element(it);

        sys[fin][i] = Epsilon(); 

    }






    return sys;

}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  resoudre_variable_the_arden
 *  Description:  aply arden's theoreme to the given line of a systeme 
 * =====================================================================================
 */
Rationnel **resoudre_variable_arden(Rationnel **ligne, int numero_variable, int n)
{
    Rationnel **res = ligne;
    Rationnel *temp; 
    bool check = false;
    int t;
    if(res[numero_variable] != NULL){
        temp = Star(res[numero_variable]);
        res[numero_variable] = NULL;
        for(t = 0; t<=n;t++){
            if(res[t] != NULL){
                res[t] = Concat(temp,res[t]);
                check = true;
            }
        }
        if(!check){
            res[n] = temp;
        }

    }
    return res;
}




/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  substituer_variable
 *  Description:  make the proper substitution of a variable on a given line
 * =====================================================================================
 */
Rationnel **substituer_variable(Rationnel **ligne, int numero_variable, Rationnel **valeur_variable, int n)
{
    Rationnel **res = ligne;
    Rationnel *temp, *temp2;
    int t;
    if(res[numero_variable]!= NULL){
        temp = res[numero_variable];
        temp2 = res[numero_variable];
        for(t = 0; t<=n; t++){
            if(valeur_variable[t] != NULL){
                temp = Concat(temp2,valeur_variable[t]);
                if(res[t] != NULL){
                    res[t] = Union(res[t],temp);
                }else{
                    res[t] = temp;
                }
                res[numero_variable] = NULL;

            }

        }

    }
    return res;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  resoudre_systeme
 *  Description:  aply arden's theoreme and make the variable's substitution to eliminate then all
 *  and return a systeme with all lines solved 
 * =====================================================================================
 */
Systeme resoudre_systeme(Systeme systeme, int n)
{
    int t;
    int counter;
    Systeme res = systeme;
    Rationnel **to_eliminate;




    /*-----------------------------------------------------------------------------
     *  solve the lines from botton to up
     *-----------------------------------------------------------------------------*/
    for(counter = 1 ; counter < n ; counter++){
        to_eliminate = resoudre_variable_arden(systeme[n-counter], n-counter, n);
        for(t = 0 ; t < n-counter; t++){
            res[t] = substituer_variable(res[t],n-counter,to_eliminate,n);



        }

    }
    /*-----------------------------------------------------------------------------
     *  solve the lines from up to botton
     *-----------------------------------------------------------------------------*/
    for(counter = 0 ; counter < n-1 ; counter++){
        to_eliminate = resoudre_variable_arden(systeme[counter], counter, n);
        for(t = 1 ; t < n; t++){
            res[n-t] = substituer_variable(res[n-t],counter,to_eliminate,n);



        }

    }




    
    res[0] =resoudre_variable_arden(res[0],0, n);



    return res;


}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  Arden
 *  Description:  creates a systeme from one automata and aply arden's theoreme 
 *  return a regex from the given automata
 * =====================================================================================
 */
Rationnel *Arden(Automate *automate)
{
    int i;
    i = get_max_etat(automate)+1;
    Systeme ard = resoudre_systeme(systeme(automate),i);





    Rationnel *res = NULL;

    Ensemble_iterateur it;
    const Ensemble * fins = get_initiaux(automate);
    for( it = premier_iterateur_ensemble( fins );
            ! iterateur_ensemble_est_vide( it );
            it = iterateur_suivant_ensemble( it )
       ){
        int fin = get_element(it);
        if(res == NULL){
            res = ard[fin][i]; 
        }else{
            res = Union(res,ard[fin][i]);
        }


    }

    return res;
}

