/******************************************************************************

	CAPITAL-GOODS MARKET OBJECT EQUATIONS
	-------------------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the capital-goods market objects in the
	K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "L1" )
/*
Work force (labor) size employed by capital-good sector
*/

v[1] = VS( LABSUPL1, "Ls" );					// available labor force
v[2] = V( "L1rd" );								// R&D labor in sector 1
v[3] = V( "L1d" );								// desired workers in sector 1
v[4] = VS( CONSECL1, "L2d" );					// desired workers in sector 2
v[5] = V( "L1shortMax" );						// max shortage allowed in sector

v[2] = min( v[2], v[1] );						// ignore demand over total labor
v[3] = min( v[3], v[1] );
v[4] = min( v[4], v[1] );

if ( v[1] - v[2] < v[3] + v[4] )				// labor shortage?
{
	v[6] = ( v[1] - v[2] ) / ( v[3] + v[4] );	// shortage factor

	if ( v[6] < 1 - v[5] )						// over cap?
		v[6] = 1 - v[5];						// shortage on cap
}
else
	v[6] = 1;									// no shortage

RESULT( v[2] + ( v[3] - v[2] ) * v[6] )


EQUATION( "entry1exit" )
/*
Rate of entry-exit of firms in capital-good sector
Perform entry and exit of firms in the capital-good sector
All relevant aggregate variables in sector must be computed before existing
firms are deleted, so all active firms in period are considered
Also updates 'cEntry1', 'cExit1', 'exit1fail'
*/

VS( CONSECL1, "K" );							// ensure canceled orders acct'd
UPDATE;											// ensure aggregates are computed

double NW10u = V( "NW10" ) * V( "PPI" ) / V( "pK0" );// minimum wealth in s. 1
double n1 = V( "n1" );							// market participation period
int F1 = V( "F1" );								// number of firms

vector < bool > quit( F1, false );				// vector of firms' quit status

WRITE( "cEntry1", 0 );							// reset exit/entry accumulators
WRITE( "cExit1", 0 );

// mark bankrupt and market-share-irrelevant firms to quit the market
h = F1;											// initial number of firms
v[1] = v[3] = i = k = 0;						// accum., counters, registers
CYCLE( cur, "Firm1" )
{
	v[4] = VS( cur, "_NW1" );					// current net wealth

	if ( v[4] < 0 || T >= VS( cur, "_t1ent" ) + n1 )// bankrupt or incumbent?
	{
		for ( v[5] = j = 0; j < n1; ++j )
			v[5] += VLS( cur, "_BC", j );		// n1 periods customer number

		if ( v[4] < 0 || v[5] <= 0 )
		{
			quit[ i ] = true;					// mark for likely exit
			--h;								// one less firm

			if ( v[5] > v[3] )					// best firm so far?
			{
				k = i;							// save firm index
				v[3] = v[5];					// and customer number
			}
		}
	}

	++i;
}

// quit candidate firms exit, except the best one if all going to quit
v[6] = i = j = 0;								// firm counters
CYCLE_SAFE( cur, "Firm1" )
{
	if ( quit[ i ] )
	{
		if ( h > 0 || i != k )					// firm must exit?
		{
			++j;								// count exits
			if ( VS( cur, "_NW1" ) < 0 )		// count bankruptcies
				++v[6];

			exit_firm( var, cur );				// del obj & collect liq. value
		}
		else
			if ( h == 0 && i == k )				// best firm must get new equity
			{
				// new equity required
				v[1] += v[7] = NW10u + VS( cur, "_Deb1" ) - VS( cur, "_NW1" );

				WRITES( cur, "_Deb1", 0 );		// reset debt
				INCRS( cur, "_Eq1", v[7] );		// add new equity
				INCRS( cur, "_NW1", v[7] );
			}
	}

	++i;
}

V( "f1rescale" );								// redistribute exiting m.s.

// replace exiting firms by entrants
entry_firm1( var, THIS, j, false );				// add entrant-firm objects

INCR( "cEntry1", v[1] );						// add cost of additional equity
WRITES( SECSTAL1, "exit1fail", v[6] / F1 );

V( "f1rescale" );								// redistribute entrant m.s.
INIT_TSEARCHT( "Firm1", i );					// prepare turbo search indexing

RESULT( ( double ) j / F1 )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A1" )
/*
Labor productivity of capital-good sector
*/
V( "PPI" );										// ensure m.s. are updated
RESULT( WHTAVE( "_Btau", "_f1" ) )


EQUATION( "D1" )
/*
Potential demand (orders) received by firms in capital-good sector
*/
RESULT( SUM( "_D1" ) )


EQUATION( "Deb1" )
/*
Total debt of capital-good sector
*/
RESULT( SUM( "_Deb1" ) )


EQUATION( "Div1" )
/*
Total dividends paid by firms in capital-good sector
*/
V( "Tax1" );									// ensure dividends are computed
RESULT( SUM( "_Div1" ) )


EQUATION( "Eq1" )
/*
Equity hold by workers/households from firms in capital-good sector
*/
RESULT( SUM( "_Eq1" ) )


EQUATION( "JO1" )
/*
Open job positions in capital-good sector
*/
RESULT( SUM( "_JO1" ) )


EQUATION( "L1d" )
/*
Total labor demand from firms in capital-good sector
Includes R&D labor
*/
RESULT( SUM( "_L1d" ) )


EQUATION( "L1dRD" )
/*
R&D labor demand from firms in capital-good sector
*/
RESULT( SUM( "_L1dRD" ) )


EQUATION( "L1rd" )
/*
Total R&D labor employed by firms in capital-good sector
Apply hard limit to sectoral labor share if L1rdMax < 1
*/
RESULT( min( V( "L1dRD" ), VS( LABSUPL1, "Ls" ) * V( "L1rdMax" ) ) )


EQUATION( "NW1" )
/*
Total net wealth (free cash) of firms in capital-good sector
*/
RESULT( SUM( "_NW1" ) )


EQUATION( "Pi1" )
/*
Total profits of capital-good sector
*/
RESULT( SUM( "_Pi1" ) )


EQUATION( "PPI" )
/*
Producer price index
*/
V( "f1rescale" );								// ensure m.s. computed/rescaled
RESULT( WHTAVE( "_p1", "_f1" ) )


EQUATION( "Q1" )
/*
Total planned output of firms in capital-good sector
*/
RESULT( SUM( "_Q1" ) )


EQUATION( "Q1e" )
/*
Total effective real output (orders) of capital-good sector
*/
RESULT( SUM( "_Q1e" ) )

EQUATION("emissions_k")
/*Aggregate emissions in capital sector
*/
RESULT( SUM( "_emissions_k") )


EQUATION( "S1" )
/*
Total sales of capital-good sector
*/
RESULT( SUM( "_S1" ) )


EQUATION( "Tax1" )
/*
Total taxes paid by firms in capital-good sector
*/
RESULT( SUM( "_Tax1" ) )


EQUATION( "W1" )
/*
Total wages paid by firms in capital-good sector
*/
RESULT( SUM( "_W1" ) )


EQUATION( "i1" )
/*
Interest paid by capital-good sector
*/
RESULT( SUM( "_i1" ) )


EQUATION( "iD1" )
/*
Interest received from deposits by capital-good sector
*/
RESULT( SUM( "_iD1" ) )


EQUATION( "imi" )
/*
Imitation success rate in capital-good sector
Also ensures all innovation/imitation is done, brochures are distributed and
learning-by-doing skills are updated
*/
SUM( "_Atau" );									// ensure innovation is done
SUM( "_NC" );									// ensure brochures distributed
RESULT( SUM( "_imi" ) / V( "F1" ) )


EQUATION( "inn" )
/*
Innovation success rate in capital-good sector
Also ensures all innovation/imitation is done, brochures are distributed and
learning-by-doing skills are updated
*/
V( "imi" );										// ensure innovation is done
RESULT( SUM( "_inn" ) / V( "F1" ) )


EQUATION( "p1avg" )
/*
Average price charged in capital-good sector
*/
RESULT( AVE( "_p1" ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "f1rescale" )
/*
Rescale market shares in capital-good sector to ensure adding to 1
To be called after market shares are changed in '_f1' and 'entry1exit'
*/

v[1] = SUM( "_f1" );							// add-up market shares

if ( ROUND( v[1], 1, 0.001 ) == 1.0 )			// ignore rounding errors
	END_EQUATION( v[1] );

v[0] = 0;										// accumulator

if ( v[1] > 0 )									// production ok?
	CYCLE( cur, "Firm1" )						// rescale to add-up to 1
	{
		v[0] += v[2] = VS( cur, "_f1" ) / v[1];	// rescaled market share
		WRITES( cur, "_f1", v[2] );				// save updated m.s.
	}
else
{
	v[2] = 1 / COUNT( "Firm1" );				// firm fair share

	CYCLE( cur, "Firm1" )						// rescale to add-up to 1
	{
		v[0] += v[2];
		WRITES( cur, "_f1", v[2] );
	}
}

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "cEntry1", "" )
/*
Cost (new equity) of firm entries in capital-good sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "cExit1", "" )
/*
Credits (returned equity) from firm exits in capital-good sector
Updated in 'entry1exit'
*/
