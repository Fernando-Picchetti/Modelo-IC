/******************************************************************************

	VINTAGE OBJECT EQUATIONS
	------------------------

	Written by Marcelo C. Pereira, University of Campinas
	Adapted by Fernando Picchetti, University of São Paulo

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the Vint objects in the model
	are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "__LdVint" )
/*
Labor required for desired utilization of vintage
*/
RESULT( V( "__toUseVint" ) * VS( GRANDPARENT, "m2" ) / V( "__Avint" ) )


EQUATION( "__RSvint" )
/*
Number of machines to scrap in vintage of firm in consumption-good sector
Positive values indicate non-economical machines but still in technical life
Negative values represent machines out of technical life to be scrapped ASAP
*/

if ( V( "__tVint" ) < T - VS( GRANDPARENT, "eta" ) )// out of technical life?
	END_EQUATION( - V( "__nVint" ) );			// scrap if not in use

VS( PARENT, "_supplier" );						// ensure supplier is selected
cur = PARENTS( SHOOKS( HOOKS( PARENT, SUPPL ) ) );// pointer to supplier
v[1] = VS( LABSUPL3, "w" );						// firm wage

// unit cost advantage of new machines
v[2] = v[1] / V( "__Avint" ) - v[1] / VS( cur, "_Atau" );

// if new machine cost is not better in absolute terms or
// payback period of replacing current vintage is over b
if ( v[2] <= 0 ||
	 VS( cur, "_p1" ) / VS( GRANDPARENT, "m2" ) / v[2] > VS( GRANDPARENT, "b" ) )
	END_EQUATION( 0 );							// nothing to scrap

RESULT( V( "__nVint" ) )						// scrap if can be replaced


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "__Qvint" )
/*
Vintage production with available workers
It is capped by the machines physical capacity
*/
VS( PARENT, "_alloc2" );						// ensure allocation is done
RESULT( V( "__Avint" ) * V( "__Lvint" ) )

EQUATION( "__Evint" )
/*
Vintage polution
*/
RESULT( V( "__Qvint" ) * V("__EAvint" ) )

/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "__Lvint", "" )
/*
Labor available for operation of vintage
Updated in '_alloc2'
*/
