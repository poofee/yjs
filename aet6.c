#include "felac.h"
double nx,ny,nz;
int nnode,ngaus,ndisp,nrefc,ncoor,nvar;
double vol,det,weigh,stif,fact,shear,r0;
int nvard[2],kdord[2],kvord[6];
double refc[12],gaus[7];
/* .... nnode ---- the number of nodes
   .... ngaus ---- the number of numerical integral points
   .... ndisp ---- the number of unknown functions
   .... nrefc ---- the number of reference coordinates
   .... nvar ---- the number of unknown varibles var
   .... refc ---- reference coordinates at integral points
   .... gaus ---- weight number at integral points
   .... nvard ---- the number of var for each unknown
   .... kdord ---- the highest differential order for each unknown
   .... kvord ---- var number at integral points for each unknown */
double coor[3];
double coorr[12];
double rctr[4],crtr[4];
/*   .... rctr ---- jacobi's matrix
     .... crtr ---- inverse matrix of jacobi's matrix */
void dshap(double (*shap)(double *,int),
           double *,double *,int,int,int);
void dcoor(double (*shap)(double *,int),
           double *,double *,double *,int,int,int);
double invm(int,double *,double *);
double inver_rc(int,int,double *,double *);
void dcoef(double (*shap)(double *,int),
           double *,double *, double *,int,int,int);
static void initial();
static void tran_coor(double *,double *,double *,double *);
static double ftran_coor(double *,int);
static void shap_az(double *,double *);
static double fshap_az(double *,int);
void shapn(int,int,int,double *,double *,double *,int,int,int);
void shapc(int,int,int,double *,double *,double *,int,int,int);
/* subroutine */
void aet6(coora,coefa,prmt,estif,emass,edamp,eload,num)
double coora[12],*coefa,*prmt,estif[36],*emass,*edamp,*eload;
int num;
/* .... coora ---- nodal coordinate value
   .... coefa ---- nodal coef value
   .... estif ---- element stiffness
   .... emass ---- element mass matrix
   .... edamp ---- element damping matrix
   .... eload ---- element load vector
   .... num   ---- element no. */
{
    double refcoor[4]= {0.0,0.0,0.0,0.0};
    double ecurlazx[7],ecurlazy[7];
    double x,y,rx,ry;
    double elump[7];
    static double raz[108],caz[18];
    /* .... store shape functions and their partial derivatives
         .... for all integral points */
    int i,j,igaus;
    int ig_az,iv,jv;
    double sigma,epsilon,fmu,fjz;
    sigma=prmt[1];
    epsilon=prmt[2];
    fmu=prmt[3];
    fjz=prmt[4];
// .... initialize the basic data
    if (num==1) initial();
    for (i=1; i<=2; ++i)
        for (j=1; j<=6; ++j)
            coorr[(i-1)*(6)+j-1]=coora[(i-1)*(6)+j-1];
    for (i=1; i<=6; ++i)
    {
        eload[i]=0.0;
        for (j=1; j<=6; ++j)
        {
            estif[(i-1)*(6)+j-1]=0.0;
        }
    }
    for (igaus=1; igaus<=ngaus; ++igaus)
    {
        for (i=1; i<=nrefc; ++i) refcoor[i]=refc[(i-1)*(6)+igaus-1];
        tran_coor(refcoor,coor,coorr,rctr);
// .... coordinate caculation by reference coordinates
// det = invm(ncoor,rctr,crtr);
        det = inver_rc(nrefc,ncoor,rctr,crtr);
        /* .... coordinate transfer from reference to original system
           .... rctr ---- jacobi's matrix
           .... crtr ---- inverse matrix of jacobi's matrix */
        x=coor[1];
        y=coor[2];
        rx=refcoor[1];
        ry=refcoor[2];
        ig_az=(igaus-1)*6*3;
        if (num>1) goto l2;
// .... the following is the shape function caculation
        shap_az(refcoor,&raz[ig_az]);
l2:
        /* .... the following is the shape function transformation
          .... from reference coordinates to original coordinates */
        shapn(nrefc,ncoor,6,&raz[ig_az],caz,crtr,1,3,3);
        weigh=det*gaus[igaus];
        for (i=1; i<=6; ++i)
        {
            ecurlazx[i] = 0.0;
            ecurlazy[i] = 0.0;
        }
        vol = 1.0;
        for (i=1; i<=6; ++i)
        {
            iv=kvord[(i-1)*(1)+1-1];
            stif=+caz[(i-1)*(3)+3-1] ;
            ecurlazx[iv]+=stif;
        }
        for (i=1; i<=6; ++i)
        {
            iv=kvord[(i-1)*(1)+1-1];
            stif=-caz[(i-1)*(3)+2-1] ;
            ecurlazy[iv]+=stif;
        }
// .... the following is the stiffness computation
        for (iv=1; iv<=6; ++iv)
        {
            for (jv=1; jv<=6; ++jv)
            {
                stif=+ecurlazx[iv]*ecurlazx[jv]*vol/fmu
                     +ecurlazy[iv]*ecurlazy[jv]*vol/fmu;
                estif[(iv-1)*(6)+jv-1]=estif[(iv-1)*(6)+jv-1]+stif*weigh;
            }
        }
// .... the following is the load vector computation
        for (i=1; i<=6; ++i)
        {
            iv=kvord[(i-1)*(1)+1-1];
            stif=+caz[(i-1)*(3)+1-1]*vol*fjz;
            eload[iv]+=stif*weigh;
        }
    }
l999:
    return;
}

static void initial()
{
// .... initial data
// .... refc ---- reference coordinates at integral points
// .... gaus ---- weight number at integral points
// .... nvard ---- the number of var for each unknown
// .... kdord ---- the highest differential order for each unknown
// .... kvord ---- var number at integral points for each unknown
    ngaus=  6;
    ndisp=  1;
    nrefc=  2;
    ncoor=  2;
    nvar =  6;
    nnode=  6;
    kdord[1]=1;
    nvard[1]=6;
    kvord[(1-1)*(1)+1-1]=1;
    kvord[(2-1)*(1)+1-1]=4;
    kvord[(3-1)*(1)+1-1]=2;
    kvord[(4-1)*(1)+1-1]=5;
    kvord[(5-1)*(1)+1-1]=3;
    kvord[(6-1)*(1)+1-1]=6;
    refc[(1-1)*(6)+1-1]=1.;
    refc[(2-1)*(6)+1-1]=0.;
    gaus[1]=0.125/3.;
    refc[(1-1)*(6)+2-1]=0.5;
    refc[(2-1)*(6)+2-1]=0.5;
    gaus[2]=0.125;
    refc[(1-1)*(6)+3-1]=0.;
    refc[(2-1)*(6)+3-1]=1.;
    gaus[3]=0.125/3.;
    refc[(1-1)*(6)+4-1]=0.;
    refc[(2-1)*(6)+4-1]=0.5;
    gaus[4]=0.125;
    refc[(1-1)*(6)+5-1]=0.;
    refc[(2-1)*(6)+5-1]=0.;
    gaus[5]=0.125/3.;
    refc[(1-1)*(6)+6-1]=0.5;
    refc[(2-1)*(6)+6-1]=0.;
    gaus[6]=0.125;
    return;
}


// void dshap(double (*shap)(double *,double *,int),double *,^shpr[][6],int,int,int);
static void shap_az(refc,shpr)
double *refc,shpr[18];
/* compute shape functions and their partial derivatives
 shapr ---- store shape functions and their partial derivatives */
{
    double (*shap)(double *,int)=&fshap_az;
// extern void dshap(shap,double *,double *,int,int,int);
    dshap(shap,refc,shpr,2,6,1);
    /* shape function and their derivatives computation
     compute partial derivatives by centered difference
     which is in the file cshap.c of felac library */
    return;
}

static double fshap_az(double *refc,int n)
// shape function caculation
{
// extern double *coor;
    double rx,ry;
    double x,y,fval;
    x=coor[1];
    y=coor[2];
    rx=refc[1];
    ry=refc[2];
    switch (n)
    {
    case 1:
        fval=+rx*(+2.*rx-1.);
        break;
    case 2:
        fval=+rx*ry*4.;
        break;
    case 3:
        fval=+ry*(+2.*ry-1.);
        break;
    case 4:
        fval=+ry*(+1.-rx-ry)*4.;
        break;
    case 5:
        fval=+(+1.-2.*rx-2.*ry)*(+1.-rx-ry);
        break;
    case 6:
        fval=+rx*(+1.-rx-ry)*4.;
        break;
//   default:
    }
    return fval;
}
static void tran_coor(double *refc,double *coor,double *coorr,double *rc)
/* compute coordinate value and jacobi's matrix rc
 by reference coordinate value */
{
    double (*shap)(double *,int)=&ftran_coor;
// extern void dcoor(shap,double *,double *,double *,
//  int,int,int);
    dcoor(shap,refc,coor,rc,2,2,1);
    /* coordinate value and their partial derivatives caculation
     compute partial derivatives by centered difference
     which is in the file cshap.c of felac library */
    return;
}

static double ftran_coor(double *refc,int n)
/* coordinate transfer function caculation */
{
    double rx,ry,fval;
    double x[7],y[7];
    int j;
    for (j=1; j<=6; ++j)
    {
        x[j]=coorr[(1-1)*(6)+j-1];
        y[j]=coorr[(2-1)*(6)+j-1];
    }
    rx=refc[1];
    ry=refc[2];
    switch (n)
    {
    case 1:
        fval=
            +(+rx*(+2.*rx-1.))*x[1]
            +(+rx*ry*4.)*x[4]
            +(+ry*(+2.*ry-1.))*x[2]
            +(+ry*(+1.-rx-ry)*4.)*x[5]
            +(+(+1.-2.*rx-2.*ry)*(+1.-rx-ry))*x[3]
            +(+rx*(+1.-rx-ry)*4.)*x[6];
        break;
    case 2:
        fval=
            +(+rx*(+2.*rx-1.))*y[1]
            +(+rx*ry*4.)*y[4]
            +(+ry*(+2.*ry-1.))*y[2]
            +(+ry*(+1.-rx-ry)*4.)*y[5]
            +(+(+1.-2.*rx-2.*ry)*(+1.-rx-ry))*y[3]
            +(+rx*(+1.-rx-ry)*4.)*y[6];
        break;
        //default:
    }
    return fval;
}

