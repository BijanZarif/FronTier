#include <iFluid.h>
#include <airfoil.h>

static boolean parachute_constr_func(POINTER,double*);
static boolean install_string_and_rotate(INTERFACE*,SURFACE*,POINTER,int);
static boolean install_string_and_rotate_w_gores(INTERFACE*,SURFACE*,
                                POINTER,int);
static boolean install_string_and_rotate_w_gores_T10(INTERFACE*,SURFACE*,
                                POINTER,int);
static boolean circle_constr_func(POINTER,double*);
static boolean cross_constr_func(POINTER,double*);
static boolean ellipse_constr_func(POINTER,double*);
static boolean wing_constr_func(POINTER, double*);
static boolean rect_constr_func(POINTER,double*);
static boolean change_mono_boundary(INTERFACE*,SURFACE*,POINTER,int);
static boolean install_strings(INTERFACE*,SURFACE*,POINTER,int);
static boolean insert_center_curve(INTERFACE*,SURFACE*,POINTER,int);
static boolean bond_intersect_with_polar_angle(double*,double,CURVE*,BOND**,
                                double*);
static void set_side_curves(double*,double*,SURFACE*,CURVE**,CURVE**,CURVE**,
                                CURVE **);

extern void initEllipticSurf(
	FILE *infile,
        Front *front,
        LEVEL_FUNC_PACK *level_func_pack)
{
	char string[100];
	AF_PARAMS *af_params = (AF_PARAMS*)front->extra2;
	int i,num_canopy;
	static ELLIP_PARAMS ellip_params;
	static STRING_PARAMS *string_params;
	static CONSTR_PARAMS constr_params;
	double *cen,*rad;
	
	num_canopy = level_func_pack->num_mono_hs;
	FT_VectorMemoryAlloc((POINTER*)&string_params,num_canopy,
                        sizeof(STRING_PARAMS));
	cen = ellip_params.cen;
	rad = ellip_params.rad;
	CursorAfterString(infile,"Enter center coordinate of the ellipsoid:");
	fscanf(infile,"%lf %lf %lf",&cen[0],&cen[1],&cen[2]);
	(void) printf("%f %f %f\n",cen[0],cen[1],cen[2]);
	CursorAfterString(infile,"Enter three radii of the ellipsoid:");
	fscanf(infile,"%lf %lf %lf",&rad[0],&rad[1],&rad[2]);
	(void) printf("%f %f %f\n",rad[0],rad[1],rad[2]);
	level_func_pack->is_mono_hs = YES;
	level_func_pack->func = ellipsoid_func;
	level_func_pack->func_params = (POINTER)&ellip_params;
	level_func_pack->constr_func = parachute_constr_func;
	level_func_pack->constr_params = (POINTER)&constr_params;
	constr_params.N[0] = constr_params.N[1] = 0.0;
	constr_params.N[2] = 1.0;
	constr_params.P[0] = cen[0];
	constr_params.P[1] = cen[1];
	CursorAfterString(infile,"Enter the height of canopy boundary:");
	fscanf(infile,"%lf",&constr_params.P[2]);
	(void) printf("%f\n",constr_params.P[2]);
	constr_params.radius = 0.0;
	CursorAfterString(infile,"Enter yes to cut a vent on canopy:");
	fscanf(infile,"%s",string);
	(void) printf("%s\n",string);
	if (string[0] == 'y' || string[0] == 'Y')
	{
	    af_params->cut_vent = YES;
	    constr_params.cen[0] = cen[0];
	    constr_params.cen[1] = cen[1];
	    CursorAfterString(infile,"Enter radius of the vent:");
	    fscanf(infile,"%lf",&constr_params.radius);
	    (void) printf("%f\n",constr_params.radius);
	}
	CursorAfterString(infile,"Enter yes to attach strings to canopy:");
	fscanf(infile,"%s",string);
	(void) printf("%s\n",string);
	if (string[0] == 'y' || string[0] == 'Y')
	{
	    level_func_pack->attach_string = YES;

	    // Yan Li
	    //level_func_pack->string_func = install_string_and_rotate;
	
	    level_func_pack->string_params = (POINTER)string_params;
	    af_params->is_parachute_system = YES;

	    // Yan Li
	    if (CursorAfterStringOpt(infile,
                        "Enter yes to attach gores on canopy:"))
            {
                fscanf(infile,"%s",string);
                (void) printf("%s\n",string);
                if (string[0] == 'y' || string[0] == 'Y')
                {
                        level_func_pack->string_func =
                                install_string_and_rotate_w_gores;
                        af_params->attach_gores = YES;
                        af_params->gore_len_fac = 1.0;	// default
                        CursorAfterStringOpt(infile,
                                "Enter gore length factor:");
                        fscanf(infile,"%lf",&(af_params->gore_len_fac));
                        (void) printf("%f\n",af_params->gore_len_fac);
                }
                else
                    level_func_pack->string_func = install_string_and_rotate;
            }
            else
                level_func_pack->string_func = install_string_and_rotate;


	    for (i = 0; i < num_canopy; ++i)
	    {
		string_params[i].cen[0] = cen[0];
		string_params[i].cen[1] = cen[1];
		string_params[i].cen[2] = cen[2];
	    	CursorAfterString(infile,"Enter number of chords:");
	    	fscanf(infile,"%d",&string_params[i].num_strings);
	    	(void) printf("%d\n",string_params[i].num_strings);
	    	CursorAfterString(infile,"Enter start angle of chord:");
	    	fscanf(infile,"%lf",&string_params[i].start_angle);
	    	(void) printf("%f\n",string_params[i].start_angle);
	    	CursorAfterString(infile,"Enter initial position of load:");
	    	fscanf(infile,"%lf %lf %lf",
					&string_params[i].coords_load[0],
					&string_params[i].coords_load[1],
					&string_params[i].coords_load[2]);
	    	(void) printf("%f %f %f\n",
					string_params[i].coords_load[0],
					string_params[i].coords_load[1],
					string_params[i].coords_load[2]);
	    	CursorAfterString(infile,"Enter rotation angles:");
	    	fscanf(infile,"%lf %lf",&string_params[i].theta,
					&string_params[i].phi);
	    	(void) printf("%f %f\n",string_params[i].theta,
					string_params[i].phi);
		string_params[i].theta *= PI/180.0;
		string_params[i].phi *= PI/180.0;
	    }
	}
	else
	    level_func_pack->attach_string = NO;
}	/* end initElllipticSurf */

extern void initParabolicSurf(
	FILE *infile,
        Front *front,
        LEVEL_FUNC_PACK *level_func_pack)
{
	char string[100];
	AF_PARAMS *af_params = (AF_PARAMS*)front->extra2;
	int i,num_canopy;
	static ELLIP_PARAMS ellip_params;
	static STRING_PARAMS *string_params;
	static CONSTR_PARAMS constr_params;
	double *cen,*rad;

	num_canopy = level_func_pack->num_mono_hs;
	FT_VectorMemoryAlloc((POINTER*)&string_params,num_canopy,
                        sizeof(STRING_PARAMS));
	cen = ellip_params.cen;
	rad = ellip_params.rad;
	CursorAfterString(infile,"Enter vertex coordinate of the paraboloid:");
	fscanf(infile,"%lf %lf %lf",&cen[0],&cen[1],&cen[2]);
	(void) printf("%f %f %f\n",cen[0],cen[1],cen[2]);
	CursorAfterString(infile,"Enter coefficients of the paraboloid:");
	fscanf(infile,"%lf %lf",&rad[0],&rad[1]);
	(void) printf("%f %f\n",rad[0],rad[1]);
	rad[0] = -rad[0]; rad[1] = -rad[1];
	level_func_pack->is_mono_hs = YES;
	level_func_pack->func = paraboloid_func;
	level_func_pack->func_params = (POINTER)&ellip_params;
	level_func_pack->constr_func = parachute_constr_func;
	level_func_pack->constr_params = (POINTER)&constr_params;
	constr_params.N[0] = constr_params.N[1] = 0.0;
	constr_params.N[2] = 1.0;
	constr_params.P[0] = cen[0];
	constr_params.P[1] = cen[1];
	CursorAfterString(infile,"Enter the height of canopy boundary:");
	fscanf(infile,"%lf",&constr_params.P[2]);
	(void) printf("%f\n",constr_params.P[2]);
	constr_params.radius = 0.0;
	CursorAfterString(infile,"Enter yes to cut a vent on canopy:");
	fscanf(infile,"%s",string);
	(void) printf("%s\n",string);
	if (string[0] == 'y' || string[0] == 'Y')
	{
	    af_params->cut_vent = YES;
	    constr_params.cen[0] = cen[0];
	    constr_params.cen[1] = cen[1];
	    CursorAfterString(infile,"Enter radius of the vent:");
	    fscanf(infile,"%lf",&constr_params.radius);
	    (void) printf("%f\n",constr_params.radius);
	}
	CursorAfterString(infile,"Enter yes to attach strings to canopy:");
	fscanf(infile,"%s",string);
	(void) printf("%s\n",string);
	if (string[0] == 'y' || string[0] == 'Y')
	{
	    level_func_pack->attach_string = YES;
	    level_func_pack->string_params = (POINTER)string_params;
	    af_params->is_parachute_system = YES;
	    if (CursorAfterStringOpt(infile,
		"Enter yes to attach gores on canopy:"))
	    {
		fscanf(infile,"%s",string);
		(void) printf("%s\n",string);
                if (string[0] == 'y' || string[0] == 'Y')
		{
		    level_func_pack->string_func = 
		    		install_string_and_rotate_w_gores_T10;
		    af_params->attach_gores = YES;
		    af_params->gore_len_fac = 1.0;
		    CursorAfterStringOpt(infile,"Enter gore length factor:");
		    fscanf(infile,"%lf",&(af_params->gore_len_fac));
		    (void) printf("%f\n",af_params->gore_len_fac);
		}
		else 
		    level_func_pack->string_func = install_string_and_rotate;
	    }
	    else
		level_func_pack->string_func = install_string_and_rotate;

	    for (i = 0; i < num_canopy; ++i)
	    {
		string_params[i].cen[0] = cen[0];
		string_params[i].cen[1] = cen[1];
		string_params[i].cen[2] = cen[2];
	    	CursorAfterString(infile,"Enter number of chords:");
	    	fscanf(infile,"%d",&string_params[i].num_strings);
	    	(void) printf("%d\n",string_params[i].num_strings);
	    	CursorAfterString(infile,"Enter start angle of chord:");
	    	fscanf(infile,"%lf",&string_params[i].start_angle);
	    	(void) printf("%f\n",string_params[i].start_angle);
	    	CursorAfterString(infile,"Enter initial position of load:");
	    	fscanf(infile,"%lf %lf %lf",
				&string_params[i].coords_load[0],
				&string_params[i].coords_load[1],
				&string_params[i].coords_load[2]);
	    	(void) printf("%f %f %f\n",
				string_params[i].coords_load[0],
				string_params[i].coords_load[1],
				string_params[i].coords_load[2]);
	    	CursorAfterString(infile,"Enter rotation angles:");
	    	fscanf(infile,"%lf %lf",&string_params[i].theta,
				&string_params[i].phi);
	    	(void) printf("%f %f\n",string_params[i].theta,
				string_params[i].phi);
		string_params[i].theta *= PI/180.0;
		string_params[i].phi *= PI/180.0;
	    }
	}
	else
	    level_func_pack->attach_string = NO;
}	/* end initParabolicSurf */
	
extern void initPlaneSurf(
	FILE *infile,
        Front *front,
        LEVEL_FUNC_PACK *level_func_pack)
{
	char string[100];
	AF_PARAMS *af_params = (AF_PARAMS*)front->extra2;
	int i,num_canopy;
	double *cen;
	static STRING_PARAMS *string_params;
	static PLANE_PARAMS plane_params;
	static CIRCLE_PARAMS circle_constr_params;
	static RECT_CONSTR_PARAMS rect_constr_params;
	static BDRY_PARAMS bdry_params;
	static WING_CONSTR_PARAMS wing_constr_params;
	static ELLIPSE_CONSTR_PARAMS ellipse_constr_params;

	
	num_canopy = level_func_pack->num_mono_hs;
	FT_VectorMemoryAlloc((POINTER*)&string_params,num_canopy,
                        sizeof(STRING_PARAMS));
	level_func_pack->func_params = (POINTER)&plane_params;
	level_func_pack->func = plane_func;
	level_func_pack->is_mono_hs = YES;
	rect_constr_params.dim = 3;
	plane_params.P[0] = plane_params.P[1] = 0.0;
	CursorAfterString(infile,"Enter the height of the plane:");
	fscanf(infile,"%lf",&plane_params.P[2]);
	(void) printf("%f\n",plane_params.P[2]);
	plane_params.N[0] = plane_params.N[1] = 0.0;
	plane_params.N[2] = 1.0;
        CursorAfterString(infile,"Enter type of plane edge:");
        fscanf(infile,"%s",string);
        (void) printf("%s\n",string);
        if (string[0] == 'c' || string[0] == 'c')
        {
	    level_func_pack->constr_params = (POINTER)&circle_constr_params;
	    level_func_pack->constr_func = circle_constr_func;
	    cen = circle_constr_params.cen;
	    CursorAfterString(infile,"Enter circle center:");
	    fscanf(infile,"%lf %lf",&circle_constr_params.cen[0],
				&circle_constr_params.cen[1]);
	    (void) printf("%f %f\n",circle_constr_params.cen[0],
				circle_constr_params.cen[1]);
	    CursorAfterString(infile,"Enter circle radius:");
	    fscanf(infile,"%lf",&circle_constr_params.R);
	    (void) printf("%f\n",circle_constr_params.R);

	    CursorAfterString(infile,"Enter yes to attach strings to canopy:");
	    fscanf(infile,"%s",string);
	    (void) printf("%s\n",string);
            if (string[0] == 'y' || string[0] == 'Y')
            {
                af_params->is_parachute_system = YES;
                level_func_pack->attach_string = YES;
                level_func_pack->string_params = (POINTER)string_params;
		if (CursorAfterStringOpt(infile,
				"Enter yes to attach gores on canopy:"))
		{
		    fscanf(infile,"%s",string);
		    (void) printf("%s\n",string);
                    if (string[0] == 'y' || string[0] == 'Y')
		    {
			level_func_pack->string_func = 
			    		install_string_and_rotate_w_gores;
			af_params->attach_gores = YES;
			af_params->gore_len_fac = 1.0; //default
			CursorAfterStringOpt(infile,
				"Enter gore length factor:");
		    	fscanf(infile,"%lf",&(af_params->gore_len_fac));
		    	(void) printf("%f\n",af_params->gore_len_fac);
		    }
		    else 
			level_func_pack->string_func = 
			    		install_string_and_rotate;
		}
		else
		    level_func_pack->string_func = install_string_and_rotate;

                for (i = 0; i < num_canopy; ++i)
                {
                    string_params[i].cen[0] = cen[0];
                    string_params[i].cen[1] = cen[1];
		    string_params[i].P[2] = plane_params.P[2];
                    CursorAfterString(infile,"Enter number of chords:");
                    fscanf(infile,"%d",&string_params[i].num_strings);
                    (void) printf("%d\n",string_params[i].num_strings);
                    CursorAfterString(infile,"Enter start angle of chord:");
                    fscanf(infile,"%lf",&string_params[i].start_angle);
                    (void) printf("%f\n",string_params[i].start_angle);
                    CursorAfterString(infile,"Enter initial position of load:");
                    fscanf(infile,"%lf %lf %lf",
                                        &string_params[i].coords_load[0],
                                        &string_params[i].coords_load[1],
                                        &string_params[i].coords_load[2]);
                    (void) printf("%f %f %f\n",
                                        string_params[i].coords_load[0],
                                        string_params[i].coords_load[1],
                                        string_params[i].coords_load[2]);
                    CursorAfterString(infile,"Enter rotation angles:");
                    fscanf(infile,"%lf %lf",&string_params[i].theta,
                                        &string_params[i].phi);
                    (void) printf("%f %f\n",string_params[i].theta,
                                        string_params[i].phi);
                    string_params[i].theta *= PI/180.0;
                    string_params[i].phi *= PI/180.0;
                }
	    }
	    else if (CursorAfterStringOpt(infile,
		     "Enter yes to change canopy boundary:"))
	    {
		fscanf(infile,"%s",string);
		(void) printf("%s\n",string);
                if (string[0] == 'y' || string[0] == 'Y')
		{
                    level_func_pack->attach_string = YES;
                    level_func_pack->string_func = change_mono_boundary;
	    	    level_func_pack->string_params = NULL;
		}
	    }
        }
        else if (string[0] == 'r' || string[0] == 'R')
        {
	    level_func_pack->constr_params = (POINTER)&rect_constr_params;
	    level_func_pack->constr_func = rect_constr_func;
	    CursorAfterString(infile,"Enter the box lower boundary:");
	    fscanf(infile,"%lf %lf %lf",&rect_constr_params.L[0],
			&rect_constr_params.L[1],&rect_constr_params.L[2]);
	    (void) printf("%f %f %f\n",rect_constr_params.L[0],
			rect_constr_params.L[1],rect_constr_params.L[2]);
	    CursorAfterString(infile,"Enter the box upper boundary:");
	    fscanf(infile,"%lf %lf %lf",&rect_constr_params.U[0],
			&rect_constr_params.U[1],&rect_constr_params.U[2]);
	    (void) printf("%f %f %f\n",rect_constr_params.U[0],
			rect_constr_params.U[1],rect_constr_params.U[2]);
	    if (CursorAfterStringOpt(infile,
			"Enter yes to change canopy boundary:"))
	    {
		fscanf(infile,"%s",string);
		(void) printf("%s\n",string);
                if (string[0] == 'y' || string[0] == 'Y')
		{
                    level_func_pack->attach_string = YES;
                    level_func_pack->string_func = change_mono_boundary;
		    bdry_params.L[0] = rect_constr_params.L[0];
		    bdry_params.L[1] = rect_constr_params.L[1];
		    bdry_params.U[0] = rect_constr_params.U[0];
		    bdry_params.U[1] = rect_constr_params.U[1];
		    for (i = 0; i < 2; ++i)
		    {
			bdry_params.lower_bdry[i] = 
			bdry_params.upper_bdry[i] = NO;	

			sprintf(string,"For direction %d",i);
			CursorAfterString(infile,string);
		    	(void) printf("%s\n",string);
			CursorAfterString(infile,
				"Enter yes to fix lower boundary:");
		    	fscanf(infile,"%s",string);
		    	(void) printf("%s\n",string);
                    	if (string[0] == 'y' || string[0] == 'Y')
			    bdry_params.lower_bdry[i] = YES;	
			else
                        {
                            bdry_params.lower_side[i] = NO_LOAD;
                            CursorAfterString(infile,"Enter load type:");
                            {
                                fscanf(infile,"%s",string);
                                (void) printf("%s\n",string);
                                switch(string[0])
                                {
                                case 'n':
                                case 'N':
                                    break;
                                case 'f':
                                case 'F':
                                    bdry_params.lower_side[i] = FREE_LOAD;
                                    break;
                                case 'r':
                                case 'R':
                                    bdry_params.lower_side[i] = RIGID_LOAD;
                                    break;
                                }
				if (bdry_params.lower_side[i] != NO_LOAD)
				{
                                    CursorAfterString(infile,
						"Enter total mass:");
                                    fscanf(infile,"%lf",
						&bdry_params.lower_mass[i]);
                                    (void) printf("%f\n",
						bdry_params.lower_mass[i]);
				}
                            }
                        }
			CursorAfterString(infile,
				"Enter yes to fix upper boundary:");
		    	fscanf(infile,"%s",string);
		    	(void) printf("%s\n",string);
                    	if (string[0] == 'y' || string[0] == 'Y')
			    bdry_params.upper_bdry[i] = YES;	
			else
                        {
                            bdry_params.upper_side[i] = NO_LOAD;
                            if (CursorAfterStringOpt(infile,"Enter load type:"))
                            {
                                fscanf(infile,"%s",string);
                                (void) printf("%s\n",string);
                                switch(string[0])
                                {
                                case 'n':
                                case 'N':
                                    break;
                                case 'f':
                                case 'F':
                                    bdry_params.upper_side[i] = FREE_LOAD;
                                    break;
                                case 'r':
                                case 'R':
                                    bdry_params.upper_side[i] = RIGID_LOAD;
                                    break;
                                }
				if (bdry_params.upper_side[i] != NO_LOAD)
				{
                                    CursorAfterString(infile,
                                    		"Enter total mass:");
                                    fscanf(infile,"%lf",
						&bdry_params.upper_mass[i]);
                                    (void) printf("%f\n",
						bdry_params.upper_mass[i]);
				}
                            }
                        }
		    }
	    	    level_func_pack->string_params = (POINTER)&bdry_params;
		}
	    }
	}
	else if (string[0] == 'x' || string[0] =='X')
	{
		level_func_pack->constr_params = (POINTER)&rect_constr_params;
	    	level_func_pack->constr_func = cross_constr_func;
	        CursorAfterString(infile,
			"Enter the box lower boundary:");
	        fscanf(infile,"%lf %lf %lf",&rect_constr_params.L[0],
			&rect_constr_params.L[1],&rect_constr_params.L[2]);
	        (void) printf("%f %f %f\n",rect_constr_params.L[0],
			rect_constr_params.L[1],rect_constr_params.L[2]);
	        CursorAfterString(infile,
			"Enter the box upper boundary:");
	        fscanf(infile,"%lf %lf %lf",&rect_constr_params.U[0],
			&rect_constr_params.U[1],&rect_constr_params.U[2]);
	        (void) printf("%f %f %f\n",rect_constr_params.U[0],
			rect_constr_params.U[1],rect_constr_params.U[2]);

		CursorAfterString(infile,
			"Enter yes to attach strings to canopy:");
		fscanf(infile,"%s",string);
		(void) printf("%s\n",string);
		if (string[0] == 'y' || string[0] == 'Y')
		{
		    af_params->is_parachute_system = YES;
	    	    level_func_pack->attach_string = YES;
	    	    level_func_pack->string_func = install_strings;
	    	    level_func_pack->string_params = (POINTER)string_params;
		    for (i = 0; i < num_canopy; ++i)
		    {
	    		CursorAfterString(infile,"Enter number of chords:");
	    		fscanf(infile,"%d",&string_params[i].num_strings);
	    		(void) printf("%d\n",string_params[i].num_strings);
	    		CursorAfterString(infile,
					"Enter initial position of load:");
	    		fscanf(infile,"%lf %lf %lf",
					&string_params[i].coords_load[0],
					&string_params[i].coords_load[1],
					&string_params[i].coords_load[2]);
	    		(void) printf("%f %f %f\n",
					string_params[i].coords_load[0],
					string_params[i].coords_load[1],
					string_params[i].coords_load[2]);
			string_params[i].cen[0] = 
					string_params[i].coords_load[0];
			string_params[i].cen[1] = 
					string_params[i].coords_load[1];
			string_params[i].L[0] = rect_constr_params.L[0];
			string_params[i].L[1] = rect_constr_params.L[1];
			string_params[i].U[0] = rect_constr_params.U[0];
			string_params[i].U[1] = rect_constr_params.U[1];
		    }
		}
	}

	    // Yan Li
	else if (string[0] == 'E'  || string[0] == 'e')
	{
	    level_func_pack->constr_params = (POINTER)&ellipse_constr_params;
	    level_func_pack->constr_func = ellipse_constr_func;
	    cen = ellipse_constr_params.cen;
	    ellipse_constr_params.dim = 3;
	    CursorAfterString(infile,"Enter ellipse center:");
	    fscanf(infile,"%lf %lf",&ellipse_constr_params.cen[0],
                                &ellipse_constr_params.cen[1]);
            (void) printf("%f %f\n",ellipse_constr_params.cen[0],
                                ellipse_constr_params.cen[1]);
            CursorAfterString(infile,"Enter ellipse radius:");
	    fscanf(infile, "%lf %lf", &ellipse_constr_params.radii[0],
				&ellipse_constr_params.radii[1]);
	    (void) printf("%f %f\n", ellipse_constr_params.radii[0],
				ellipse_constr_params.radii[1]);

	    CursorAfterString(infile,
                        "Enter yes to attach strings to canopy:");
	    fscanf(infile,"%s",string);
            (void) printf("%s\n",string);
	    if (string[0] == 'y' || string[0] == 'Y')
	    {
                af_params->is_parachute_system = YES;
                level_func_pack->attach_string = YES;
                level_func_pack->string_params = (POINTER)string_params;
                if (CursorAfterStringOpt(infile,
                                "Enter yes to attach gores on canopy:"))
                {
                    fscanf(infile,"%s",string);
                    (void) printf("%s\n",string);
                    if (string[0] == 'y' || string[0] == 'Y')
                    {
                        level_func_pack->string_func =
                                    install_string_and_rotate_w_gores;
                        af_params->attach_gores = YES;
                        af_params->gore_len_fac = 1.0; //default
                        CursorAfterStringOpt(infile,
                                "Enter gore length factor:");
                        fscanf(infile,"%lf",&(af_params->gore_len_fac));
                        (void) printf("%f\n",af_params->gore_len_fac);
                    }
                    else
                        level_func_pack->string_func =
                                    install_string_and_rotate;
                }
                else
                    level_func_pack->string_func = install_string_and_rotate;

                for (i = 0; i < num_canopy; ++i)
                {
                    string_params[i].cen[0] = cen[0];
                    string_params[i].cen[1] = cen[1];
                    string_params[i].P[2] = plane_params.P[2];
                    CursorAfterString(infile,"Enter number of chords:");
                    fscanf(infile,"%d",&string_params[i].num_strings);
                    (void) printf("%d\n",string_params[i].num_strings);
                    CursorAfterString(infile,"Enter start angle of chord:");
                    fscanf(infile,"%lf",&string_params[i].start_angle);
                    (void) printf("%f\n",string_params[i].start_angle);
                    CursorAfterString(infile,"Enter initial position of load:");
                    fscanf(infile,"%lf %lf %lf",
                                        &string_params[i].coords_load[0],
                                        &string_params[i].coords_load[1],
                                        &string_params[i].coords_load[2]);
                    (void) printf("%f %f %f\n",
                                        string_params[i].coords_load[0],
                                        string_params[i].coords_load[1],
                                        string_params[i].coords_load[2]);
                    CursorAfterString(infile,"Enter rotation angles:");
                    fscanf(infile,"%lf %lf",&string_params[i].theta,
                                        &string_params[i].phi);
                    (void) printf("%f %f\n",string_params[i].theta,
                                        string_params[i].phi);
                    string_params[i].theta *= PI/180.0;
                    string_params[i].phi *= PI/180.0;
                }
            }
            else if (CursorAfterStringOpt(infile,
                                "Enter yes to change canopy boundary:"))
            {
                fscanf(infile,"%s",string);
                (void) printf("%s\n",string);
                if (string[0] == 'y' || string[0] == 'Y')
                {
                    level_func_pack->attach_string = YES;
                    level_func_pack->string_func = change_mono_boundary;
                    level_func_pack->string_params = NULL;
                }
            }
	}
	else if (string[0] == 'W' || string[0] == 'w')
	{
	    level_func_pack->constr_params = (POINTER)&wing_constr_params;
	    level_func_pack->constr_func = wing_constr_func;
	    wing_constr_params.dim = 3;
	    CursorAfterString(infile,"Enter x symmetric axis:");
	    fscanf(infile, "%lf", &wing_constr_params.x_sym);
	    (void) printf("%f", wing_constr_params.x_sym);
	    CursorAfterString(infile, "Enter y constraint:");
	    fscanf(infile, "%lf", &wing_constr_params.y_constraint);
	    (void) printf("%f", wing_constr_params.y_constraint);
	    CursorAfterString(infile, "Enter x deviation:");
	    fscanf(infile, "%lf", &wing_constr_params.x_devi);
	    (void) printf("%f", wing_constr_params.x_devi);
	    CursorAfterString(infile, "Enter ellipse radius:");
	    fscanf(infile, "%lf %lf", &wing_constr_params.radius[0],
			&wing_constr_params.radius[1]);
	    (void) printf("%f %f", wing_constr_params.radius[0],
			wing_constr_params.radius[1]);

	    CursorAfterString(infile, "Enter yes to insert center curve:");
	    fscanf(infile,"%s",string);
	    (void) printf("%s\n",string);
	    if (string[0] == 'y' || string[0] == 'Y')
	    {
		CursorAfterString(infile, "Enter start point of curve:");
		fscanf(infile, "%lf %lf", 
			&wing_constr_params.cen_curve_start[0],
                        &wing_constr_params.cen_curve_start[1]);
		(void) printf("%f %f", 
			wing_constr_params.cen_curve_start[0], 
			wing_constr_params.cen_curve_start[1]);
                CursorAfterString(infile, "Enter end point of curve:");
                fscanf(infile, "%lf %lf", 
                        &wing_constr_params.cen_curve_end[0],
                        &wing_constr_params.cen_curve_end[1]);
                (void) printf("%f %f", 
                        wing_constr_params.cen_curve_end[0], 
                        wing_constr_params.cen_curve_end[1]);

		level_func_pack->attach_string = YES;
		level_func_pack->string_func = insert_center_curve;
		level_func_pack->string_params = 
			(POINTER) &wing_constr_params;
	    }
	}
}	/* end initPlaneSurf */


static boolean circle_constr_func(
        POINTER params,
        double *coords)
{
        CIRCLE_PARAMS *circle_constr_params = (CIRCLE_PARAMS*)params;
	double *cen = circle_constr_params->cen;
	double R = circle_constr_params->R;
	double r;
	
	r = sqrt(sqr(coords[0] - cen[0]) + sqr(coords[1] - cen[1]));
	if ( r <= R) return YES;
	else return NO;
}	/* end circle_constr_func */

static boolean rect_constr_func(
        POINTER params,
        double *coords)
{
        RECT_CONSTR_PARAMS *rect_constr_params = (RECT_CONSTR_PARAMS*)params;
	int i;
	int dim = rect_constr_params->dim;
	double *L = rect_constr_params->L;
	double *U = rect_constr_params->U;
	for (i = 0; i < dim-1; ++i)
	{
	    if (coords[i] < L[i] || coords[i] > U[i])
		return NO;
	}
	return YES;
}	/* end rect_constr_func */

static boolean cross_constr_func(
        POINTER params,
        double *coords)
{
        RECT_CONSTR_PARAMS *rect_constr_params = (RECT_CONSTR_PARAMS*)params;
	int i;
	int dim = rect_constr_params->dim;
	double *L = rect_constr_params->L;
	double *U = rect_constr_params->U;
	double LL[3],UU[3];
	int counter1,counter2;

	LL[0] = L[1]; LL[1] = L[0]; LL[2] = L[2];
	UU[0] = U[1]; UU[1] = U[0]; UU[2] = U[2];

	counter1 = counter2 = 0;
	for (i = 0; i < dim-1; ++i)
        {
            if (coords[i] > L[i] && coords[i] < U[i])
		counter1++;
        }
	for (i = 0; i < dim-1; ++i)
	{
	    if (coords[i] > LL[i] && coords[i] < UU[i])
		counter2++;
	}
	if (counter1 == 2 || counter2 == 2)
	    return YES;
        return NO;
}	/* end cross_constr_func */


// Yan Li
static boolean ellipse_constr_func(
	POINTER params,
	double *coords)
{
	ELLIPSE_CONSTR_PARAMS *ellipse_constr_params = 
			(ELLIPSE_CONSTR_PARAMS*)params;
	int i;
	int dim = ellipse_constr_params->dim;
	double *cen = ellipse_constr_params->cen;
	double *radii = ellipse_constr_params->radii;
	double r = 0;

	for (i = 0; i < dim - 1; ++i)
	    r += sqr((coords[i] - cen[i]) / radii[i]);

	if (r < 1)
	    return YES;
	return NO;
}

// Yan Li
static boolean wing_constr_func(
	POINTER params,
	double *coords)
{
	WING_CONSTR_PARAMS *wing_constr_params = 
			(WING_CONSTR_PARAMS*) params;

	double x_sym = wing_constr_params->x_sym;
	double y_constraint = wing_constr_params->y_constraint;
	double x_devi = wing_constr_params->x_devi;
	double *radius = wing_constr_params->radius;
	double x, y, r = 0;

	if (coords[1] > y_constraint)
	    return NO;

	if (coords[0] < x_sym)
	    x = coords[0] - (x_sym - x_devi);
	else
	    x = coords[0] - (x_sym + x_devi);

	y = coords[1] - y_constraint;

	r = sqr(x / radius[0]) + sqr(y / radius[1]);

	if (r <1)
	    return YES;
	return NO; 
	
}

// Yan Li
static boolean insert_center_curve(
        INTERFACE *intfc,
        SURFACE *surf,
        POINTER params,
        int ip)
{
	CURVE **c, *center_curve;
	POINT **pts;
	int num_curves, num_points;
	int i, j;
	boolean find_start, find_end;
	NODE *start_node, *end_node;
	double *start, *end;
	WING_CONSTR_PARAMS *wing_constr_params = (WING_CONSTR_PARAMS*) params;
	double dis1, dis2;

	start = wing_constr_params->cen_curve_start;
	end = wing_constr_params->cen_curve_end;

        num_curves = FT_NumOfSurfCurves(surf);
        FT_VectorMemoryAlloc((POINTER*)&c,num_curves,sizeof(CURVE*));
        FT_ArrayOfSurfCurves(surf,c);

        for (i = 0; i < num_curves; ++i)
	{
	    if (hsbdry_type(c[i]) != MONO_COMP_HSBDRY)
		continue;
	    num_points = FT_NumOfCurvePoints(c[i]);
	    FT_VectorMemoryAlloc((POINTER*)&pts,num_points,sizeof(POINT*));
	    FT_ArrayOfCurvePoints(c[i],pts);

	    find_start = NO;
	    find_end = NO;
	    for (j = 0; j < num_points; ++j)
	    {
		dis1 = sqr(Coords(pts[j])[0] - start[0]) + 
		    sqr(Coords(pts[j])[1] - start[1]);
		dis2 = sqr(Coords(pts[j])[0] - end[0]) + 
		    sqr(Coords(pts[j])[1] - end[1]);

		if (sqrt(dis1) < 1e-10)
		{
		    start_node = make_node(pts[j]);
		    find_start = YES;
		}
		else if (sqrt(dis2) < 1e-10)
		{
		    end_node = make_node(pts[j]);
		    find_end = YES;
		}
		


		if (find_start && find_end)
		{
		    center_curve = make_curve(0,0, start_node, end_node);
		    hsbdry_type(center_curve) = STRING_HSBDRY;
		    break;
		}
	    }

	    FT_FreeThese(1,pts);
	}
	FT_FreeThese(1,c);

	return YES;

}

static boolean parachute_constr_func(
        POINTER params,
        double *coords)
{
        CONSTR_PARAMS *constr_params = (CONSTR_PARAMS*)params;
        double *N,v[MAXD],dist;
        int i;
	double *cen = constr_params->cen;

        N = constr_params->N;
        for (i = 0; i < 3; ++i)
            v[i] = coords[i] - constr_params->P[i];
        dist = Dot3d(N,v);
        if (dist < 0.0) return NO;
	dist = sqrt(sqr(coords[0] - cen[0]) + sqr(coords[1] - cen[1]));
	return (dist > constr_params->radius) ? YES : NO;
}       /* end parachute_constr_func */

static boolean install_string_and_rotate(
	INTERFACE *intfc,
	SURFACE *surf,
	POINTER params,
	int ip)
{
	CURVE **c,*canopy_bdry,*curve;
	POINT **pts,**string_pts;
	STRING_PARAMS *tmp_params = (STRING_PARAMS*)params;
	STRING_PARAMS *string_params = tmp_params+ip;
	double *cen = string_params->cen,*cload,coords[MAXD];
	double ave_radius_sqr,max_radius_sqr;
	int i,j,k,num_curves,num_points,num_strings;
	int nb;
	double *string_angle,start_angle,d_angle;
	double theta1,theta2,d1,d2,rot_theta,rot_phi;
	NODE *nload,**string_nodes;
	boolean node_moved;
	AF_NODE_EXTRA *extra;
	double spacing,dir[MAXD],*h = computational_grid(intfc)->h;
	BOND *b;

	num_strings = string_params->num_strings;
	start_angle = string_params->start_angle;
	rot_theta = string_params->theta;
	rot_phi   = string_params->phi;
	cload = string_params->coords_load;
	d_angle = 2*PI/num_strings;

	canopy_bdry = NULL;
	nload = make_node(Point(cload));
	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	extra->af_node_type = LOAD_NODE;
	nload->extra = (POINTER)extra;

	FT_VectorMemoryAlloc((POINTER*)&string_angle,num_strings,
				sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&string_pts,num_strings,
				sizeof(POINT*));
	FT_VectorMemoryAlloc((POINTER*)&string_nodes,num_strings,
				sizeof(NODE*));
	for (i = 0; i < num_strings; ++i)
	{
	    string_angle[i] = start_angle + i*d_angle;
	    if (string_angle[i] > 2.0*PI) 
		string_angle[i] -= 2.0*PI;
	}

	num_curves = FT_NumOfSurfCurves(surf);
	FT_VectorMemoryAlloc((POINTER*)&c,num_curves,sizeof(CURVE*));
	FT_ArrayOfSurfCurves(surf,c);

	max_radius_sqr = 0.0;
	for (i = 0; i < num_curves; ++i)
	{
	    if (hsbdry_type(c[i]) != MONO_COMP_HSBDRY)
		continue;
	    num_points = FT_NumOfCurvePoints(c[i]);
	    FT_VectorMemoryAlloc((POINTER*)&pts,num_points,sizeof(POINT*));
	    FT_ArrayOfCurvePoints(c[i],pts);
	    ave_radius_sqr = 0.0;
	    for (j = 0; j < num_points; ++j)
	    {
		ave_radius_sqr += sqr(Coords(pts[j])[0] - cen[0]) +
			      sqr(Coords(pts[j])[1] - cen[1]);
	    }
	    ave_radius_sqr /= (double)num_points;
	    if (ave_radius_sqr > max_radius_sqr)
	    {
		max_radius_sqr = ave_radius_sqr;
		canopy_bdry = c[i];
	    }
	    FT_FreeThese(1,pts);
	}
	FT_FreeThese(1,c);

	for (j = 0; j < num_strings; ++j)
        {
            if (!bond_intersect_with_polar_angle(cen,string_angle[j],
                                canopy_bdry,&b,coords))
            {
                printf("Cannot find intersecting bond\n");
                clean_up(ERROR);
            }
            string_pts[j] = Point(coords);
            insert_point_in_bond(string_pts[j],b,canopy_bdry);
        }

	node_moved = NO;
	for (i = 0; i < num_strings; ++i)
	{
	    canopy_bdry = FT_CurveOfPoint(intfc,string_pts[i],&b);
	    if (is_closed_curve(canopy_bdry) && !node_moved)
	    {
		move_closed_loop_node(canopy_bdry,b);
	        string_nodes[i] = FT_NodeOfPoint(intfc,string_pts[i]);
	    	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    	extra->af_node_type = STRING_NODE;
	    	string_nodes[i]->extra = (POINTER)extra;
		node_moved = YES;
	    }
	    else
	    {
	    	c = split_curve(string_pts[i],b,canopy_bdry,0,0,0,0);
	    	string_nodes[i] = FT_NodeOfPoint(intfc,string_pts[i]);
	    	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    	extra->af_node_type = STRING_NODE;
	    	string_nodes[i]->extra = (POINTER)extra;
	    }
	}
	for (i = 0; i < num_strings; ++i)
	{
	    curve = make_curve(0,0,string_nodes[i],nload);
	    hsbdry_type(curve) = STRING_HSBDRY;
	    spacing = separation(string_nodes[i]->posn,nload->posn,3);
	    for (j = 0; j < 3; ++j)
		dir[j] = (Coords(nload->posn)[j] - 
			Coords(string_nodes[i]->posn)[j])/spacing;
	    nb = (int)spacing/(1.1*h[0]);
	    spacing /= (double)nb;
	    b = curve->first;
	    for (j = 1; j < nb; ++j)
	    {
	    	for (k = 0; k < 3; ++k)
		    coords[k] = Coords(string_nodes[i]->posn)[k] + 
					j*dir[k]*spacing;
		insert_point_in_bond(Point(coords),b,curve);
		b = b->next;
	    }
	}
	FT_FreeThese(3,string_angle,string_pts,string_nodes);

	num_points = FT_NumOfSurfPoints(surf);
	FT_VectorMemoryAlloc((POINTER*)&pts,num_points,sizeof(POINT*));
	FT_ArrayOfSurfPoints(surf,pts);
	rotate_point_with_polar_angle(nload->posn,cload,rot_phi,rot_theta,YES);
	for (i = 0; i < num_points; ++i)
	    rotate_point_with_polar_angle(pts[i],cload,rot_phi,rot_theta,NO);

	FT_FreeThese(1,pts);
	return YES;
}	/* end install_string_and_rotate */

static boolean install_string_and_rotate_w_gores(
	INTERFACE *intfc,
	SURFACE *surf,
	POINTER params,
	int ip)
{
	CURVE **c,*canopy_bdry,*curve;
	POINT *gore_vertex,**pts,**string_pts;
	STRING_PARAMS *tmp_params = (STRING_PARAMS*)params;
	STRING_PARAMS *string_params = tmp_params+ip;
	double *cen = string_params->cen,*cload,coords[MAXD];
	double ccanopy[MAXD];	/* like cload, center on canopy */
	double ave_radius_sqr,max_radius_sqr;
	int i,j,k,num_curves,num_points,num_strings;
	int nb;
	double *string_angle;
	double start_angle,d_angle,rot_theta,rot_phi;
	NODE *nload,**string_nodes;
	NODE *ncanopy;	/* like nload */
	boolean node_moved;
	AF_NODE_EXTRA *extra;
	double spacing,dir[MAXD],*h = computational_grid(intfc)->h;
	BOND *b;
	double v1[MAXD],v2[MAXD],v3[MAXD];

	printf("Entering install_string_and_rotate_w_gores()\n");
	num_strings = string_params->num_strings;
	start_angle = string_params->start_angle;
	rot_theta = string_params->theta;
	rot_phi   = string_params->phi;
	cload = string_params->coords_load;
	d_angle = 2*PI/num_strings;

	canopy_bdry = NULL;
	nload = make_node(Point(cload));
	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	extra->af_node_type = LOAD_NODE;
	nload->extra = (POINTER)extra;

	FT_VectorMemoryAlloc((POINTER*)&string_angle,num_strings,
				sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&string_pts,num_strings,
				sizeof(POINT*));
	FT_VectorMemoryAlloc((POINTER*)&string_nodes,num_strings,
				sizeof(NODE*));
	for (i = 0; i < num_strings; ++i)
	{
	    string_angle[i] = start_angle + i*d_angle;
	    if (string_angle[i] > 2.0*PI) 
		string_angle[i] -= 2.0*PI;
	}

	num_curves = FT_NumOfSurfCurves(surf);
	FT_VectorMemoryAlloc((POINTER*)&c,num_curves,sizeof(CURVE*));
	FT_ArrayOfSurfCurves(surf,c);

	max_radius_sqr = 0.0;
	for (i = 0; i < num_curves; ++i)
	{
	    if (hsbdry_type(c[i]) != MONO_COMP_HSBDRY)
		continue;
	    num_points = FT_NumOfCurvePoints(c[i]);
	    FT_VectorMemoryAlloc((POINTER*)&pts,num_points,sizeof(POINT*));
	    FT_ArrayOfCurvePoints(c[i],pts);
	    ave_radius_sqr = 0.0;
	    for (j = 0; j < num_points; ++j)
	    {
		ave_radius_sqr += sqr(Coords(pts[j])[0] - cen[0]) +
			      sqr(Coords(pts[j])[1] - cen[1]);
	    }
	    ave_radius_sqr /= (double)num_points;
	    if (ave_radius_sqr > max_radius_sqr)
	    {
		max_radius_sqr = ave_radius_sqr;
		canopy_bdry = c[i];
	    }
	    FT_FreeThese(1,pts);
	}
	FT_FreeThese(1,c);

	for (j = 0; j < num_strings; ++j)
	{
	    if (!bond_intersect_with_polar_angle(cen,string_angle[j],
                                canopy_bdry,&b,coords))
            {
                printf("Cannot find intersecting bond\n");
                clean_up(ERROR);
            }
	    string_pts[j] = Point(coords);
	    insert_point_in_bond(string_pts[j],b,canopy_bdry);
	}

	node_moved = NO;
	for (i = 0; i < num_strings; ++i)
	{
	    canopy_bdry = FT_CurveOfPoint(intfc,string_pts[i],&b);
	    if (is_closed_curve(canopy_bdry) && !node_moved)
	    {
		move_closed_loop_node(canopy_bdry,b);
	        string_nodes[i] = FT_NodeOfPoint(intfc,string_pts[i]);
	    	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    	extra->af_node_type = STRING_NODE;
	    	string_nodes[i]->extra = (POINTER)extra;
		node_moved = YES;
	    }
	    else
	    {
	    	c = split_curve(string_pts[i],b,canopy_bdry,0,0,0,0);
	    	string_nodes[i] = FT_NodeOfPoint(intfc,string_pts[i]);
	    	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    	extra->af_node_type = STRING_NODE;
	    	string_nodes[i]->extra = (POINTER)extra;
	    }
	}

	/* Insert gore vertex on canopy */
	ccanopy[0] = cen[0]; 
	ccanopy[1] = cen[1]; 
	ccanopy[2] = string_params->P[2]; 
	gore_vertex = insert_point_in_surface(2,ccanopy,surf);
	if (gore_vertex != NULL)
	{
	    ncanopy = make_node(gore_vertex);
	    FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    extra->af_node_type = GORE_NODE;
	    ncanopy->extra = (POINTER)extra;
	}
	else
	{
	    (void) printf("Insert point in canopy failed!\n");
	    clean_up(ERROR);
	}
	v1[0] = v1[1] = 0.0; 	v1[2] = 1.0;

	for (i = 0; i < num_strings; ++i)
	{
	    curve = make_curve(0,0,string_nodes[i],nload);
	    hsbdry_type(curve) = STRING_HSBDRY;
	    spacing = separation(string_nodes[i]->posn,nload->posn,3);
	    for (j = 0; j < 3; ++j)
		dir[j] = (Coords(nload->posn)[j] - 
			Coords(string_nodes[i]->posn)[j])/spacing;
	    nb = (int)spacing/(1.1*h[0]);
	    spacing /= (double)nb;
	    b = curve->first;
	    for (j = 1; j < nb; ++j)
	    {
	    	for (k = 0; k < 3; ++k)
		    coords[k] = Coords(string_nodes[i]->posn)[k] + 
					j*dir[k]*spacing;
		insert_point_in_bond(Point(coords),b,curve);
		b = b->next;
	    }
	    direction_vector(Coords(string_nodes[i]->posn),
				Coords(ncanopy->posn),v2,3);
	    Cross3d(v1,v2,v3);
	    curve = insert_curve_in_surface(v3,string_nodes[i],
					ncanopy,surf);
	    hsbdry_type(curve) = GORE_HSBDRY;
	}
	FT_FreeThese(3,string_angle,string_pts,string_nodes);

	num_points = FT_NumOfSurfPoints(surf);
	FT_VectorMemoryAlloc((POINTER*)&pts,num_points,sizeof(POINT*));
	FT_ArrayOfSurfPoints(surf,pts);
	rotate_point_with_polar_angle(nload->posn,cload,rot_phi,rot_theta,YES);
	for (i = 0; i < num_points; ++i)
	    rotate_point_with_polar_angle(pts[i],cload,rot_phi,rot_theta,NO);

	FT_FreeThese(1,pts);
	return YES;
}	/* end install_string_and_rotate_w_gores */

static boolean install_string_and_rotate_w_gores_T10(
	INTERFACE *intfc,
	SURFACE *surf,
	POINTER params,
	int ip)
{
	CURVE **c,*canopy_bdry,*vent_bdry,*string_curve,*gore_curve;
	POINT **pts,**string_pts,**vent_pts,*gore_vertex;
	STRING_PARAMS *tmp_params = (STRING_PARAMS*)params;
	STRING_PARAMS *string_params = tmp_params+ip;
	double *cen = string_params->cen,*cload,coords[MAXD];
	double ccanopy[MAXD];	/* like cload, center on canopy */
	int i,j,k,num_curves,num_points,num_strings;
	int nb;
	double *string_angle,start_angle,d_angle;
	double theta1,theta2,d1,d2,rot_theta,rot_phi;
	NODE *nload,**string_nodes, **vent_nodes;
	NODE *ncanopy;	/* like nload */
	boolean str_node_moved,vnt_node_moved;
	AF_NODE_EXTRA *extra;
	double spacing,dir[MAXD],*h = computational_grid(intfc)->h;
	BOND *b;
	CURVE *mono_c[10];
	double ave_radius_sqr[10];
	double v1[MAXD], v2[MAXD], v3[MAXD];

	num_strings = string_params->num_strings;
	start_angle = string_params->start_angle;
	rot_theta = string_params->theta;
	rot_phi   = string_params->phi;
	cload = string_params->coords_load;
	d_angle = 2*PI/num_strings;

	canopy_bdry = vent_bdry = NULL;
	nload = make_node(Point(cload));
	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	extra->af_node_type = LOAD_NODE;
	nload->extra = (POINTER)extra;

	FT_VectorMemoryAlloc((POINTER*)&string_angle,num_strings,
				sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&string_pts,num_strings,
				sizeof(POINT*));
	FT_VectorMemoryAlloc((POINTER*)&vent_pts,num_strings,
				sizeof(POINT*));
	FT_VectorMemoryAlloc((POINTER*)&string_nodes,num_strings,
				sizeof(NODE*));
	FT_VectorMemoryAlloc((POINTER*)&vent_nodes, num_strings,
				sizeof(NODE*));

	for (i = 0; i < num_strings; ++i)
	{
	    string_angle[i] = start_angle + i*d_angle;
	    if (string_angle[i] > 2.0*PI) 
		string_angle[i] -= 2.0*PI;
	}

	num_curves = FT_NumOfSurfCurves(surf);
	FT_VectorMemoryAlloc((POINTER*)&c,num_curves,sizeof(CURVE*));
	FT_ArrayOfSurfCurves(surf,c);

	k = 0;
	for (i = 0; i < num_curves; ++i)
	{
	    if (hsbdry_type(c[i]) != MONO_COMP_HSBDRY)
		continue;
	    mono_c[k] = c[i];
	    num_points = FT_NumOfCurvePoints(c[i]);
	    FT_VectorMemoryAlloc((POINTER*)&pts,num_points,sizeof(POINT*));
	    FT_ArrayOfCurvePoints(c[i],pts);
	    ave_radius_sqr[k] = 0.0;
	    for (j = 0; j < num_points; ++j)
	    {
		ave_radius_sqr[k] += sqr(Coords(pts[j])[0] - cen[0]) +
			      sqr(Coords(pts[j])[1] - cen[1]);
	    }
	    ave_radius_sqr[k] /= (double)num_points;
	    FT_FreeThese(1,pts);
	    if (++k > 2)
	    {
		(void) printf("More than two bdry curves at the beginning!\n");
		clean_up(ERROR);
	    }
	}
	if (k == 2)     /* There exist vent */
        {
            canopy_bdry = (ave_radius_sqr[0] > ave_radius_sqr[1]) ? mono_c[0]
                                : mono_c[1];
            vent_bdry = (ave_radius_sqr[0] > ave_radius_sqr[1]) ? mono_c[1]
                                : mono_c[0];
        }
        else    /* No vent */
	{
            canopy_bdry = mono_c[0];
	    ccanopy[0] = cen[0]; ccanopy[1] = cen[1]; ccanopy[2] = cen[2];
	    gore_vertex = insert_point_in_surface(2,ccanopy,surf);
	    ncanopy = make_node(gore_vertex);
	    FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    extra->af_node_type = GORE_NODE;
	    ncanopy->extra = (POINTER)extra;
	}
	FT_FreeThese(1,c);

	for (i = 0; i < num_strings; ++i)
	{
	    if (!bond_intersect_with_polar_angle(cen,string_angle[i],
                        	canopy_bdry,&b,coords))
            {
                printf("Cannot find intersecting bond\n");
                clean_up(ERROR);
            }
	    string_pts[i] = Point(coords);
	    insert_point_in_bond(string_pts[i],b,canopy_bdry);
	    /* If vent exist */
	    if (vent_bdry != NULL)
	    {
	    	if (!bond_intersect_with_polar_angle(cen,string_angle[i],
                        	vent_bdry,&b,coords))
            	{
                    printf("Cannot find intersecting bond\n");
                    clean_up(ERROR);
            	}
	    	vent_pts[i] = Point(coords);
	    	insert_point_in_bond(vent_pts[i],b,vent_bdry);
	    }
	}

	str_node_moved = vnt_node_moved = NO;
	for (i = 0; i < num_strings; ++i)
	{
	    canopy_bdry = FT_CurveOfPoint(intfc,string_pts[i],&b);
	    if (is_closed_curve(canopy_bdry) && !str_node_moved)
	    {
		move_closed_loop_node(canopy_bdry,b);
	        string_nodes[i] = FT_NodeOfPoint(intfc,string_pts[i]);
	    	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    	extra->af_node_type = STRING_NODE;
	    	string_nodes[i]->extra = (POINTER)extra;
		str_node_moved = YES;
	    }
	    else
	    {
	    	c = split_curve(string_pts[i],b,canopy_bdry,0,0,0,0);
	    	string_nodes[i] = FT_NodeOfPoint(intfc,string_pts[i]);
	    	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    	extra->af_node_type = STRING_NODE;
	    	string_nodes[i]->extra = (POINTER)extra;
	    }
	}
	if (vent_bdry != NULL)
	{
	    for (i = 0; i < num_strings; ++i)
	    {
	    	vent_bdry = FT_CurveOfPoint(intfc,vent_pts[i],&b);
	    	if (is_closed_curve(vent_bdry) && !vnt_node_moved)
	    	{
		    move_closed_loop_node(vent_bdry,b);
	            vent_nodes[i] = FT_NodeOfPoint(intfc,vent_pts[i]);
	    	    FT_ScalarMemoryAlloc((POINTER*)&extra,
				sizeof(AF_NODE_EXTRA));
	    	    extra->af_node_type = GORE_NODE;
	    	    vent_nodes[i]->extra = (POINTER)extra;
		    vnt_node_moved = YES;
	    	}
	    	else
	    	{
	    	    c = split_curve(vent_pts[i],b,vent_bdry,0,0,0,0);
	    	    vent_nodes[i] = FT_NodeOfPoint(intfc,vent_pts[i]);
	    	    FT_ScalarMemoryAlloc((POINTER*)&extra,
				sizeof(AF_NODE_EXTRA));
	    	    extra->af_node_type = GORE_NODE;
	    	    vent_nodes[i]->extra = (POINTER)extra;
	    	}
	    }
	}

	v1[0] = v1[1] = 0.0;	v1[2] = 1.0;
	for (i = 0; i < num_strings; ++i)
	{
	    string_curve = make_curve(0,0,string_nodes[i],nload);
	    hsbdry_type(string_curve) = STRING_HSBDRY;
	    spacing = separation(string_nodes[i]->posn,nload->posn,3);
	    for (j = 0; j < 3; ++j)
		dir[j] = (Coords(nload->posn)[j] - 
			Coords(string_nodes[i]->posn)[j])/spacing;
	    nb = (int)spacing/(1.1*h[0]);
	    spacing /= (double)nb;
	    b = string_curve->first;
	    for (j = 1; j < nb; ++j)
	    {
	    	for (k = 0; k < 3; ++k)
		    coords[k] = Coords(string_nodes[i]->posn)[k] + 
					j*dir[k]*spacing;
		insert_point_in_bond(Point(coords),b,string_curve);
		b = b->next;
	    }
	    if (vent_bdry != NULL)
	    	direction_vector(Coords(string_nodes[i]->posn),
				Coords(vent_nodes[i]->posn),v2,3);
	    else
	    	direction_vector(Coords(string_nodes[i]->posn),
				Coords(gore_vertex),v2,3);
	    Cross3d(v1,v2,v3);
	    if (vent_bdry != NULL)
	    	gore_curve = insert_curve_in_surface(v3,string_nodes[i],
				vent_nodes[i],surf);
	    else
	    	gore_curve = insert_curve_in_surface(v3,string_nodes[i],
				ncanopy,surf);
	    hsbdry_type(gore_curve) = GORE_HSBDRY;
	}
	FT_FreeThese(5, string_angle, string_pts,string_nodes,vent_pts, 
				vent_nodes);

	num_points = FT_NumOfSurfPoints(surf);
	FT_VectorMemoryAlloc((POINTER*)&pts,num_points,sizeof(POINT*));
	FT_ArrayOfSurfPoints(surf,pts);
	rotate_point_with_polar_angle(nload->posn,cload,rot_phi,rot_theta,YES);
	for (i = 0; i < num_points; ++i)
	    rotate_point_with_polar_angle(pts[i],cload,rot_phi,rot_theta,NO);

	FT_FreeThese(1,pts);
	return YES;
}	/* end install_string_and_rotate_w_gores_T10 */

static boolean install_strings(
	INTERFACE *intfc,
	SURFACE *surf,
	POINTER params,
	int ip)
{
	CURVE **c,*canopy_bdry,**string_curves;
	POINT **pts,**string_pts;
	BOND **bonds;
	STRING_PARAMS *tmp_params = (STRING_PARAMS*)params;
	STRING_PARAMS *string_params = tmp_params+ip;
	double *cen = string_params->cen,*cload,coords[MAXD];
	double ave_radius_sqr,max_radius_sqr;
	int i,j,k,num_curves,num_points,num_bonds,num_strings;
	int nb;
	double *string_angle,start_angle,d_angle;
	double theta1,theta2,d1,d2,rot_theta,rot_phi;
	NODE *nload,**string_nodes;
	boolean node_moved;
	AF_NODE_EXTRA *extra;
	double spacing,dir[MAXD],*h = computational_grid(intfc)->h;
	BOND *b;
	double *U,*L, width;

	num_strings = string_params->num_strings;
	start_angle = string_params->start_angle;
	rot_theta = string_params->theta;
	rot_phi   = string_params->phi;
	cload = string_params->coords_load;
	d_angle = 2*PI/num_strings;
	U = string_params->U;
	L = string_params->L;
	width = U[1]-L[1];

	canopy_bdry = NULL;
	nload = make_node(Point(cload));
	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	extra->af_node_type = LOAD_NODE;
	nload->extra = (POINTER)extra;

	FT_VectorMemoryAlloc((POINTER*)&string_angle,num_strings,
				sizeof(double));
	FT_VectorMemoryAlloc((POINTER*)&string_pts,num_strings,
				sizeof(POINT*));
	FT_VectorMemoryAlloc((POINTER*)&string_nodes,num_strings,
				sizeof(NODE*));
	FT_VectorMemoryAlloc((POINTER*)&string_curves,num_strings,
				sizeof(CURVE*));

	for (i = 0; i < 5; ++i)
	{
	    coords[0] = L[1] + i*(width/4.0); coords[1] = L[0];
	    string_angle[i] = plane_angle(cen,coords);
	    coords[0] = U[0]; coords[1] = L[1] + i*(width/4.0);
	    string_angle[i+5] = plane_angle(cen,coords);
	    coords[0] = L[1] + i*(width/4.0); coords[1] = U[0];
	    string_angle[i+10] = plane_angle(cen,coords);
	    coords[0] = L[0]; coords[1] = L[1] + i*(width/4.0);
	    string_angle[i+15] = plane_angle(cen,coords);
	}

	for (i = 0; i < num_strings; ++i)
        {
            if (string_angle[i] > 2.0*PI) 
        	string_angle[i] -= 2.0*PI;
        }

	num_curves = FT_NumOfSurfCurves(surf);
	FT_VectorMemoryAlloc((POINTER*)&c,num_curves,sizeof(CURVE*));
	FT_ArrayOfSurfCurves(surf,c);

	max_radius_sqr = 0.0;
	for (i = 0; i < num_curves; ++i)
	{
	    if (hsbdry_type(c[i]) != MONO_COMP_HSBDRY)
		continue;
	    num_points = FT_NumOfCurvePoints(c[i]);
	    FT_VectorMemoryAlloc((POINTER*)&pts,num_points,sizeof(POINT*));
	    FT_ArrayOfCurvePoints(c[i],pts);
	    ave_radius_sqr = 0.0;
	    for (j = 0; j < num_points; ++j)
	    {
		ave_radius_sqr += sqr(Coords(pts[j])[0] - cen[0]) +
			      sqr(Coords(pts[j])[1] - cen[1]);
	    }
	    ave_radius_sqr /= (double)num_points;
	    if (ave_radius_sqr > max_radius_sqr)
	    {
		max_radius_sqr = ave_radius_sqr;
		canopy_bdry = c[i];
	    }
	    FT_FreeThese(1,pts);
	}
	FT_FreeThese(1,c);

	num_bonds = FT_NumOfCurveBonds(canopy_bdry);
	FT_VectorMemoryAlloc((POINTER*)&bonds,num_bonds,sizeof(BOND*));
	FT_ArrayOfCurveBonds(canopy_bdry,bonds);
	for (i = 0; i < num_bonds; ++i)
	{
	    theta1 = plane_angle(cen,Coords(bonds[i]->start));
	    theta2 = plane_angle(cen,Coords(bonds[i]->end));
	    if (fabs(theta1 - theta2) > PI) 
	    {
		if (theta2 > theta1) theta2 -= 2*PI;
		else theta1 -= 2*PI;
	    }
	    for (j = 0; j < num_strings; ++j)
	    {
		if (within_interval(theta1,theta2,string_angle[j]) ||
		    within_interval(theta1,theta2,string_angle[j]-2*PI))
		{
	    	    d1 = distance_between_positions(cen,Coords(bonds[i]->start),2);
	    	    d2 = distance_between_positions(cen,Coords(bonds[i]->end),2);
		    d1 = 0.5*(d1 + d2);
		    coords[0] = cen[0] + d1*cos(string_angle[j]);
		    coords[1] = cen[1] + d1*sin(string_angle[j]);
		    coords[2] = 0.5*(Coords(bonds[i]->start)[2] + 
					Coords(bonds[i+1]->end)[2]);
		    string_pts[j] = Point(coords);
	    	    insert_point_in_bond(string_pts[j],bonds[i],canopy_bdry);
		} 
	    }
	}
	FT_FreeThese(1,bonds);

	node_moved = NO;
	for (i = 0; i < num_strings; ++i)
	{
	    canopy_bdry = FT_CurveOfPoint(intfc,string_pts[i],&b);
	    if (is_closed_curve(canopy_bdry) && !node_moved)
	    {
		move_closed_loop_node(canopy_bdry,b);
	        string_nodes[i] = FT_NodeOfPoint(intfc,string_pts[i]);
	    	FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    	extra->af_node_type = STRING_NODE;
	    	string_nodes[i]->extra = (POINTER)extra;
		node_moved = YES;
		continue;
	    }
	    c = split_curve(string_pts[i],b,canopy_bdry,0,0,0,0);
	    string_nodes[i] = FT_NodeOfPoint(intfc,string_pts[i]);
	    FT_ScalarMemoryAlloc((POINTER*)&extra,sizeof(AF_NODE_EXTRA));
	    extra->af_node_type = STRING_NODE;
	    string_nodes[i]->extra = (POINTER)extra;
	}
	for (i = 0; i < num_strings; ++i)
	{
	    string_curves[i] = make_curve(0,0,string_nodes[i],nload);
	    hsbdry_type(string_curves[i]) = STRING_HSBDRY;
	    spacing = separation(string_nodes[i]->posn,nload->posn,3);
	    for (j = 0; j < 3; ++j)
		dir[j] = (Coords(nload->posn)[j] - 
			Coords(string_nodes[i]->posn)[j])/spacing;
	    nb = (int)spacing/(1.1*h[0]);
	    spacing /= (double)nb;
	    b = string_curves[i]->first;
	    for (j = 1; j < nb; ++j)
	    {
	    	for (k = 0; k < 3; ++k)
		    coords[k] = Coords(string_nodes[i]->posn)[k] + 
					j*dir[k]*spacing;
		insert_point_in_bond(Point(coords),b,string_curves[i]);
		b = b->next;
	    }
	}
	FT_FreeThese(4,string_angle,string_pts,string_nodes,string_curves);
	return YES;
}	/* end install_strings */

static boolean change_mono_boundary(
	INTERFACE *intfc,
	SURFACE *surf,
	POINTER params,
	int ip)
{

	if (params != NULL)
	{
	    BDRY_PARAMS *bdry_params = (BDRY_PARAMS*)params;
	    double *L,*U;
	    boolean *lower_bdry = bdry_params->lower_bdry;
	    boolean *upper_bdry = bdry_params->upper_bdry;
	    CURVE *cside00,*cside01,*cside10,*cside11;
	    L = bdry_params->L;
	    U = bdry_params->U;
	    set_side_curves(L,U,surf,&cside00,&cside01,&cside10,&cside11);
	    if (lower_bdry[0] == YES)
		hsbdry_type(cside00) = FIXED_HSBDRY;
	    if (upper_bdry[0] == YES)
		hsbdry_type(cside01) = FIXED_HSBDRY;
	    else
            {
                static C_PARAMS c_params;
                int npts = FT_NumOfCurvePoints(cside01);
                c_params.load_type = bdry_params->upper_side[0];
                c_params.load_mass = bdry_params->upper_mass[0];
                c_params.point_mass = bdry_params->upper_mass[0]/npts;
                c_params.dir = 0;
                cside01->extra = (POINTER)&c_params;
            }
	    if (lower_bdry[1] == YES)
		hsbdry_type(cside10) = FIXED_HSBDRY;
	    if (upper_bdry[1] == YES)
		hsbdry_type(cside11) = FIXED_HSBDRY;
	}
	else
	{
	    /* Circular surface, only one boundary */
	    CURVE **c,*cbdry;
	    for (c = surf->pos_curves; c && *c; ++c)
	    {
		if (hsbdry_type(*c) == MONO_COMP_HSBDRY)
		    hsbdry_type(*c) = FIXED_HSBDRY;
	    }
	    for (c = surf->neg_curves; c && *c; ++c)
	    {
		if (hsbdry_type(*c) == MONO_COMP_HSBDRY)
		    hsbdry_type(*c) = FIXED_HSBDRY;
	    }
	}
	return YES;
}	/* end change_mono_boundary */

static void rotate_point(
	POINT *p,
	double *center,			// Rotation center
	double phi,			// Polar angle
	double theta,			// Azimuthal angle
	boolean first)
{
	static double roty_matrix[3][3];
	static double rotz_matrix[3][3];
	double pt[3];
	int i,j;

	if (first == YES)
	{
	    /* Set up rotation matrix */
	    roty_matrix[0][0] = cos(theta);
	    roty_matrix[0][1] = 0.0;
	    roty_matrix[0][2] = sin(theta);
	    roty_matrix[1][0] = 0.0;
	    roty_matrix[1][1] = 1.0;
	    roty_matrix[1][2] = 0.0;
	    roty_matrix[2][0] = -sin(theta);
	    roty_matrix[2][1] = 0.0;
	    roty_matrix[2][2] = cos(theta);
	    
	    rotz_matrix[0][0] = cos(phi);
	    rotz_matrix[0][1] = -sin(phi);
	    rotz_matrix[0][2] = 0.0;
	    rotz_matrix[1][0] = sin(phi);
	    rotz_matrix[1][1] = cos(phi);
	    rotz_matrix[1][2] = 0.0;
	    rotz_matrix[2][0] = 0.0;
	    rotz_matrix[2][1] = 0.0;
	    rotz_matrix[2][2] = 1.0;
	}
	for (i = 0; i < 3; i++)
	    Coords(p)[i] -= center[i];
	for (i = 0; i < 3; i++)
	{
	    pt[i] = 0.0; 
	    for (j = 0; j < 3; j++)
	    {
		pt[i] += roty_matrix[i][j]*Coords(p)[j];
	    }
	}
	for (i = 0; i < 3; i++)
	{
	    Coords(p)[i] = 0.0;
	    for (j = 0; j < 3; j++)
	    {
		Coords(p)[i] += rotz_matrix[i][j]*pt[j];
	    }
	}
	for (i = 0; i < 3; i++)
	    Coords(p)[i] += center[i];
}	/* end rotate_point */

static void set_side_curves(
	double *L,
	double *U,
	SURFACE *s,
	CURVE **cside00,
	CURVE **cside01,
	CURVE **cside10,
	CURVE **cside11)
{
	BOND *b00,*b01,*b10,*b11;
	POINT *p00,*p01,*p10,*p11;
	double crds[MAXD];
	CURVE **c,*cbdry,*curves[4];
	int i,nc;
	for (c = s->pos_curves; c && *c; ++c)
	{
	    if (hsbdry_type(*c) == MONO_COMP_HSBDRY)
		cbdry = *c;
	}
	for (c = s->neg_curves; c && *c; ++c)
	{
	    if (hsbdry_type(*c) == MONO_COMP_HSBDRY)
		cbdry = *c;
	}

	crds[0] = L[0];	crds[1] = L[1];
	closest_point_on_curve(&p00,&b00,crds,cbdry);
	crds[0] = U[0];	crds[1] = L[1];
	closest_point_on_curve(&p01,&b01,crds,cbdry);
	crds[0] = L[0];	crds[1] = U[1];
	closest_point_on_curve(&p10,&b10,crds,cbdry);
	crds[0] = U[0];	crds[1] = U[1];
	closest_point_on_curve(&p11,&b11,crds,cbdry);

	change_node_of_closed_curve(p00,cbdry);

	c = split_curve(p01,b01,cbdry,0,0,0,0);
	nc = 0;
	curves[nc++] = c[0];
	curves[nc++] = c[1];

	for (i = 0; i < nc; ++i)
	{
	    if (point_on_curve(p10,&b10,curves[i]))
	    {
		c = split_curve(p10,b10,curves[i],0,0,0,0);
		curves[i] = c[0];
	    }
	}
	curves[nc++] = c[1];
	for (i = 0; i < nc; ++i)
	{
	    if (point_on_curve(p11,&b11,curves[i]))
	    {
		c = split_curve(p11,b11,curves[i],0,0,0,0);
		curves[i] = c[0];
	    }
	}
	curves[nc++] = c[1];
	*cside00 = *cside01 = *cside10 = *cside11 = NULL;
	for (i = 0; i < nc; ++i)
	{
	    if (point_on_curve(p00,&b00,curves[i]) &&
		point_on_curve(p10,&b10,curves[i]))
		*cside00 = curves[i];
	    if (point_on_curve(p01,&b01,curves[i]) &&
		point_on_curve(p11,&b11,curves[i]))
		*cside01 = curves[i];
	    if (point_on_curve(p00,&b00,curves[i]) &&
		point_on_curve(p01,&b01,curves[i]))
		*cside10 = curves[i];
	    if (point_on_curve(p10,&b10,curves[i]) &&
		point_on_curve(p11,&b11,curves[i]))
		*cside11 = curves[i];
	}
}	/* end set_side_curves */

static boolean bond_intersect_with_polar_angle(
        double *cen,
        double angle,
        CURVE *c,
        BOND **the_b,
        double *coords)
{
        BOND *bond;
        double theta1,theta2;
        double *p1,*p2;
        double k1,k2;

        for (bond = c->first; bond != NULL; bond = bond->next)
        {
            theta1 = plane_angle(cen,Coords(bond->start));
            theta2 = plane_angle(cen,Coords(bond->end));
            if (fabs(theta1 - theta2) > PI)
            {
                if (theta2 > theta1) theta2 -= 2*PI;
                else theta1 -= 2*PI;
            }

            if (within_interval(theta1,theta2,angle) ||
                within_interval(theta1,theta2,angle-2*PI))
            {
                *the_b = bond;
                p1 = Coords(bond->start);
                p2 = Coords(bond->end);

                k1 = tan(angle);
                k2 = (p2[1] - p1[1])/(p2[0] - p1[0]);

                if( fabs(p1[0] - p2[0]) < 1e-6 )  //k2 = inf
                {
                    coords[0] = p1[0];
                    coords[1] = k1*(coords[0] - cen[0]) + cen[1];
                    coords[2] = (p1[2] - p2[2])*(coords[1] - p2[1])/
				(p1[1] - p2[1])+ p2[2];
                }
                else if( fabs(angle - 0.5 * PI) < 1e-6 ) //k1 = inf
                {
                    coords[0] = cen[0];
                    coords[1] = k2*(coords[0] - p1[0]) + p1[1];
                    coords[2] = (p1[2] - p2[2])*(coords[0] - p2[0])/
				(p1[0] - p2[0])+ p2[2];
                }
                else
                {
                    coords[0] = (k1*cen[0] - k2*p1[0] + p1[1] - cen[1])/
				(k1 - k2);
                    coords[1] = k1*(coords[0] - cen[0]) + cen[1];
                    coords[2] = (p1[2] - p2[2])*(coords[0] - p2[0])/
				(p1[0] - p2[0])+ p2[2];
                }
                return YES;
            }
        }
        return NO;
}       /* bond_intersect_with_polar_angle */

