/******************************************************************************

	CONSUMER-GOODS MARKET OBJECT EQUATIONS
	--------------------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the consumer-goods market objects in the
	K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "D2" )
/*
Demand fulfilled by firms in consumption-good sector
Update '_D2', '_l2'
*/

k = V( "F2" );									// number of firms
v[1] = VS( PARENT, "Cd" );						// nominal demand

// create and fill temporary share and supply vectors & initialize firm demand
dblVecT f2( k ), p2( k ), sup2( k );

j = 0;
CYCLE( cur, "Firm2" )
{
	sup2[ j ] = VS( cur, "_Q2e" ) + VLS( cur, "_N", 1 );// firm available supply
	f2[ j ] = VS( cur, "_f2" );					// firm market share
	p2[ j ] = VS( cur, "_p2" );					// firm price
	WRITES( cur, "_D2", 0 );					// demand fulfilled accumulator
	WRITES( cur, "_l2", 0 );					// assume no unsatisfied demand
	++j;
}

// cycle through firms until all demand is allocated or no more product to sell
v[0] = i = 0;									// fulfilled demand accumulator
while ( v[1] > 0.01 )
{
	v[2] = v[1];								// remaining unallocated $ demand
	v[3] = j = 0;								// shares yet unallocated
	CYCLE( cur, "Firm2" )
	{
		if ( f2[ j ] > 0 )						// firm has demand to supply
		{
			if ( sup2[ j ] > 0 )				// product to supply?
			{
				v[4] = v[1] * f2[ j ];			// firm $ demand allocation
				v[5] = v[4] / p2[ j ];			// firm # demand allocation

				if ( v[5] <= sup2[ j ] )		// can supply all demanded?
				{
					INCRS( cur, "_D2", v[5] );	// supply all demanded
					v[0] += v[5];				// accumulate to total # demand
					v[2] -= v[4];				// discount from desired $ demand
					v[3] += f2[ j ];			// save share yet to allocate
					sup2[ j ] -= v[5];			// make supplied # unavailable
				}
				else
				{
					if ( i == 0 )				// unsatisfied demand metric
						WRITES( cur, "_l2", v[5] - sup2[ j ] );

					INCRS( cur, "_D2", sup2[ j ] );// supply all # available
					v[0] += sup2[ j ];			// accumulate to total # demand
					v[2] -= sup2[ j ] * p2[ j ];// discount from desired $ demand
					f2[ j ] = sup2[ j ] = 0;	// nothing else to supply
				}
			}
			else
				f2[ j ] = 0;					// nothing else to supply
		}

		++j;
	}

	if ( v[3] > 0 )								// unallocated shares remaining?
		for ( j = 0; j < k; ++j )
			f2[ j ] /= v[3];					// rescale remaining firms
	else
		break;									// nothing else to supply

	v[1] = v[2];								// update unallocated
	++i;
}

RESULT( v[0] )


EQUATION( "entry2exit" )
/*
Rate of entry-exit of firms in consumer-good sector
Perform entry and exit of firms in the consumer-good sector
All relevant aggregate variables in sector must be computed before existing
firms are deleted, so all active firms in period are considered
Also updates 'cEntry2', 'cExit2', 'exit2fail'
*/

SUM( "_D2d" );									// desired demand before chg
UPDATE;											// ensure aggregates are computed

double NW20u = V( "NW20" ) * VS( CAPSECL1, "PPI" ) / VS( CAPSECL1, "pK0" );
												// minimum wealth in sector 2
double f2min = V( "f2min" );					// min market share in sector 2
double n2 = V( "n2" );							// market participation period
int F2 = V( "F2" );								// current number of firms

vector < bool > quit( F2, false );				// vector of firms' quit status

WRITE( "cEntry2", 0 );							// reset exit/entry accumulators
WRITE( "cExit2", 0 );

// mark bankrupt and market-share-irrelevant incumbent firms to quit the market
h = F2;											// initial number of firms
v[1] = v[3] = i = k = 0;						// accum., counters, registers
CYCLE( cur, "Firm2" )
{
	v[4] = VS( cur, "_NW2" );					// current net wealth

	if ( v[4] < 0 || VS( cur, "_life2cycle" ) > 0 )// bankrupt or incumbent?
	{
		for ( v[5] = j = 0; j < n2; ++j )
			v[5] += VLS( cur, "_f2", j ) / n2;	// n2 periods avg. market share

		if ( v[4] < 0 || v[5] < f2min )
		{
			quit[ i ] = true;					// mark for likely exit
			--h;								// one less firm

			if ( v[5] > v[3] )					// best firm so far?
			{
				k = i;							// save firm index
				v[3] = v[5];					// and market share
			}
		}
	}

	++i;
}

// quit candidate firms exit, except the best one if all going to quit
v[6] = i = j = 0;								// firm counters
CYCLE_SAFE( cur, "Firm2" )
{
	if ( quit[ i ] )
	{
		if ( h > 0 || i != k )					// firm must exit?
		{
			++j;								// count exits
			if ( VS( cur, "_NW2" ) < 0 )		// count bankruptcies
				++v[6];

			exit_firm( var, cur );				// del obj & collect liq. val.
		}
		else
			if ( h == 0 && i == k )				// best firm must get new equity
			{
				// new equity required
				v[1] += v[7] = NW20u + VS( cur, "_Deb2" ) - VS( cur, "_NW2" );

				WRITES( cur, "_Deb2", 0 );		// reset debt
				INCRS( cur, "_Eq2", v[7] );		// add new equity
				INCRS( cur, "_NW2", v[7] );
			}
	}

	++i;
}

V( "f2rescale" );								// redistribute exiting m.s.

// replace exiting firms by entrants
entry_firm2( var, THIS, j, false );				// add entrant-firm objects

INCR( "cEntry2", v[1] );						// add cost of additional equity
WRITES( SECSTAL1, "exit2fail", v[6] / F2 );

V( "f2rescale" );								// redistribute entrant m.s.
V( "firm2maps" );								// update firm mapping vectors

RESULT( ( double ) j / F2 )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A2" )
/*
Machine-level weighted-average labor productivity of consumption-good sector
*/
V( "CPI" );										// ensure m.s. are updated
RESULT( WHTAVE( "_A2", "_f2" ) )


EQUATION( "CI" )
/*
Total canceled investment in consumption-good sector
*/
RESULT( SUM( "_CI" ) )


EQUATION( "CPI" )
/*
Consumer price index
*/
V( "f2rescale" );								// ensure m.s. computed/rescaled
RESULT( WHTAVE( "_p2", "_f2" ) )


EQUATION( "D2d" )
/*
Desired demand for firms in consumption-good sector
*/
RESULT( VS( PARENT, "Cd" ) / V( "CPI" ) )


EQUATION( "D2e" )
/*
Demand expectation of firms in consumer-good sector
*/
RESULT( SUM( "_D2e" ) )


EQUATION( "Deb2" )
/*
Total debt of consumption-good sector
*/
RESULT( SUM( "_Deb2" ) )


EQUATION( "Div2" )
/*
Total dividends paid by firms in consumption-good sector
*/
V( "Tax2" );									// ensure dividends are computed
RESULT( SUM( "_Div2" ) )


EQUATION( "EI" )
/*
Total expansion investment in consumption-good sector
*/
V( "CI" );										// ensure cancellations acct'd
RESULT( SUM( "_EI" ) )


EQUATION( "Eavg" )
/*
Weighted average competitiveness of firms in consumption-good sector
*/

v[0] = 0;
CYCLE( cur, "Firm2" )
	if ( VS( cur, "_life2cycle" ) > 0 )			// consider only non-entrants
		v[0] += VS( cur, "_E" ) * VLS( cur, "_f2", 1 );
												// compute weighted average
RESULT( v[0] )


EQUATION( "Eq2" )
/*
Equity hold by workers/households from firms in consumption-good sector
*/
RESULT( SUM( "_Eq2" ) )


EQUATION( "Id" )
/*
Total desired investment in terms of output capacity (real terms)
Don't recompute 'SI'/'EI' at this stage, to wait for order cancellations
*/
RESULT( SUM( "_SI" ) + SUM( "_EI" ) )


EQUATION( "Inom" )
/*
Aggregated investment (nominal/currency terms)
*/
RESULT( SUM( "_Inom" ) )


EQUATION( "Ireal" )
/*
Aggregated real investment (in initial prices terms)
*/
RESULT( ( V( "SI" ) + V( "EI" ) ) / V( "m2" ) * VS( CAPSECL1, "pK0" ) )


EQUATION( "JO2" )
/*
Open job positions in consumption-good sector
*/
RESULT( SUM( "_JO2" ) )


EQUATION( "K" )
/*
Total capital accumulated by firms in consumption-good sector
After new machine orders are delivered
*/
RESULT( SUM( "_K" ) )


EQUATION( "Kd" )
/*
Total desired capital stock of firms in consumption-good sector
*/
RESULT( SUM( "_Kd" ) )


EQUATION( "Knom" )
/*
Total capital (nominal/money terms) in consumption-good sector
*/
RESULT( SUM( "_Knom" ) )


EQUATION( "L2" )
/*
Work force (labor) size in consumption-good sector
*/
v[1] = VS( LABSUPL1, "Ls" ) - VS( CAPSECL1, "L1" );// available labor force
v[2] = V( "L2d" );								// desired workers in sector 2
RESULT( min( v[2], v[1] ) )						// pick up to available


EQUATION( "L2d" )
/*
Total labor demand from firms in consumption-good sector
*/
RESULT( SUM( "_L2d" ) )


EQUATION( "N" )
/*
Total inventories (real terms)
*/
RESULT( SUM( "_N" ) )


EQUATION( "NW2" )
/*
Total net wealth (free cash) of firms in consumption-good sector
*/
RESULT( SUM( "_NW2" ) )


EQUATION( "Pi2" )
/*
Total profits of consumer-good sector
*/
RESULT( SUM( "_Pi2" ) )


EQUATION( "Q2" )
/*
Total planned output before labor/financial constraints in consumption-good sector
*/
RESULT( SUM( "_Q2" ) )


EQUATION( "Q2d" )
/*
Total desired real output of consumption-good sector
*/
RESULT( SUM( "_Q2d" ) )


EQUATION( "Q2e" )
/*
Total effective output of firms in consumption-good sector
*/
RESULT( SUM( "_Q2e" ) )


EQUATION( "emissions_c" )
/*
Aggregate emissions of firms in consumption-good sector
*/
RESULT( SUM( "_emissions_c" ) )


EQUATION( "Q2p" )
/*
Potential production with current machines in consumption-good sector
*/
RESULT( SUM( "_Q2p" ) )


EQUATION( "Q2u" )
/*
Capacity utilization of consumption-good sector
*/
v[1] = V( "Q2p" );
RESULT( v[1] > 0 ? V( "Q2e" ) / v[1] : 0 )


EQUATION( "S2" )
/*
Total sales of consumption-good sector
*/
RESULT( SUM( "_S2" ) )


EQUATION( "SI" )
/*
Total substitution investment in consumption-good sector
*/
V( "CI" );										// ensure cancellations acct'd
RESULT( SUM( "_SI" ) )


EQUATION( "Tax2" )
/*
Total taxes paid by firms in consumption-good sector
*/
RESULT( SUM( "_Tax2" ) )


EQUATION( "W2" )
/*
Total wages paid by all firms in sector 2
*/
RESULT( SUM( "_W2" ) )


EQUATION( "c2" )
/*
Planned average unit cost in consumption-good sector
*/
v[1] = V( "Q2" );
RESULT( v[1] > 0 ? WHTAVE( "_c2", "_Q2" ) / v[1] : CURRENT )


EQUATION( "c2e" )
/*
Planned average unit cost in consumption-good sector
*/
v[1] = V( "Q2e" );
RESULT( v[1] > 0 ? WHTAVE( "_c2e", "_Q2e" ) / v[1] : CURRENT )


EQUATION( "dCPI" )
/*
Consumer price index inflation (change) rate
*/
RESULT( V( "CPI" ) / VL( "CPI", 1 ) - 1 )


EQUATION( "dCPIb" )
/*
Consumer price index inflation (change) rate
*/
RESULT( mov_avg_bound( THIS, "CPI", VS( PARENT, "mLim" ), VS( PARENT, "mPer" ) ) )


EQUATION( "dNnom" )
/*
Change in total nominal inventories (currency terms)
*/
RESULT( SUM( "_dNnom" ) )


EQUATION( "i2" )
/*
Interest paid by consumption-good sector
*/
RESULT( SUM( "_i2" ) )


EQUATION( "iD2" )
/*
Interest received from deposits by consumption-good sector
*/
RESULT( SUM( "_iD2" ) )


EQUATION( "l2avg" )
/*
Average unfilled demand in consumption-good sector
*/
v[1] = SUM( "_life2cycle" );
RESULT( v[1] > 0 ? WHTAVE( "_l2", "_life2cycle" ) / v[1] : CURRENT )


EQUATION( "oldVint" )
/*
Oldest vintage (ID) in use in period by any firm in consumption-good sector
*/
RESULT( MIN( "_oldVint" ) )


EQUATION( "p2avg" )
/*
Average price charged in consumption-good sector
*/
v[1] = SUM( "_life2cycle" );
RESULT( v[1] > 0 ? WHTAVE( "_p2", "_life2cycle" ) / v[1] : CURRENT )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "f2rescale" )
/*
Rescale market shares in consumption-good sector to ensure adding to 1
To be called after market shares are changed in '_f2' and 'entry2exit'
*/

v[1] = SUM( "_f2" );							// add-up market shares

if ( ROUND( v[1], 1, 0.001 ) == 1.0 )			// ignore rounding errors
	END_EQUATION( v[1] );

v[0] = 0;										// accumulator

if ( v[1] > 0 )									// production ok?
	CYCLE( cur, "Firm2" )						// rescale to add-up to 1
	{
		v[0] += v[2] = VS( cur, "_f2" ) / v[1];	// rescaled market share
		WRITES( cur, "_f2", v[2] );				// save updated m.s.
	}
else
{
	v[2] = 1 / COUNT( "Firm2" );				// firm fair share

	CYCLE( cur, "Firm2" )						// rescale to add-up to 1
	{
		v[0] += v[2];
		WRITES( cur, "_f2", v[2] );
	}
}

RESULT( v[0] )


EQUATION( "firm2maps" )
/*
Updates the static maps of firms in consumption-good sector
Only to be called if firm objects in sector 2 are created or destroyed
*/

// clear vectors
EXEC_EXTS( PARENT, countryE, firm2map, clear );
EXEC_EXTS( PARENT, countryE, firm2ptr, clear );

i = 0;											// firm index in vector
CYCLE( cur, "Firm2" )							// do for all firms in sector 2
{
	EXEC_EXTS( PARENT, countryE, firm2ptr, push_back, cur );// pointer to firm
	EXEC_EXTS( PARENT, countryE, firm2map, insert, // save in firm's map
			   firmPairT( ( int ) VS( cur, "_ID2" ), cur ) );

	++i;
}

RESULT( i )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "cEntry2", "" )
/*
Cost (new equity) of firm entries in consumption-good sector
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "cExit2", "" )
/*
Credits (returned equity) from firm exits in consumption-good sector
Updated in 'entry2exit'
*/
