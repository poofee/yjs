#include "felac.h"
double nx,ny,nz;
int nnode,ngaus,ndisp,nrefc,ncoor,nvar;
double vol,det,weigh,stif,fact,shear,r0;
int nvard[3],kdord[3],kvord[16];
double refc[8],gaus[5];
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
double coorr[8];
double coefr[4];
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
static void shap_hx(double *,double *);
static double fshap_hx(double *,int);
static void shap_hy(double *,double *);
static double fshap_hy(double *,int);
static void coef_shap(double *,double *,double *,double *);
static double fcoef_shap(double *,int);
void shapn(int,int,int,double *,double *,double *,int,int,int);
void shapc(int,int,int,double *,double *,double *,int,int,int);
/* subroutine */
void beq4g2(coora,coefa,prmt,estif,emass,edamp,eload,num)
double coora[8],coefa[4],*prmt,estif[64],*emass,*edamp,*eload;
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
    double coef[2];
    double az;
    double coefd[5],coefc[5];
    double x,y,rx,ry;
    double elump[9];
    static double rhx[48],rhy[48],chx[12],chy[12];
    /* .... store shape functions and their partial derivatives
         .... for all integral points */
    int i,j,igaus;
    int ig_hx,ig_hy,iv,jv;
    double fmu,curlazx,curlazy;
    fmu=prmt[1];
// .... initialize the basic data
    if (num==1) initial();
    for (i=1; i<=2; ++i)
        for (j=1; j<=4; ++j)
            coorr[(i-1)*(4)+j-1]=coora[(i-1)*(4)+j-1];
    for (i=1; i<=1; ++i)
        for (j=1; j<=4; ++j)
            coefr[(i-1)*(4)+j-1]=coefa[(i-1)*(4)+j-1];
    for (i=1; i<=8; ++i)
    {
        emass[i]=0.0;
        elump[i]=0.0;
        eload[i]=0.0;
        for (j=1; j<=8; ++j)
        {
            estif[(i-1)*(8)+j-1]=0.0;
        }
    }
    for (igaus=1; igaus<=ngaus; ++igaus)
    {
        for (i=1; i<=nrefc; ++i) refcoor[i]=refc[(i-1)*(4)+igaus-1];
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
        coef_shap(refcoor,coef,coefr,coefd);
// .... compute coef functions and their partial derivatives
        az=coef[1];
        ig_hx=(igaus-1)*4*3;
        ig_hy=(igaus-1)*4*3;
        if (num>1) goto l2;
// .... the following is the shape function caculation
        shap_hx(refcoor,&rhx[ig_hx]);
        shap_hy(refcoor,&rhy[ig_hy]);
l2:
        /* .... the following is the shape function transformation
          .... from reference coordinates to original coordinates */
        shapn(nrefc,ncoor,4,&rhx[ig_hx],chx,crtr,1,3,3);
        shapn(nrefc,ncoor,4,&rhy[ig_hy],chy,crtr,1,3,3);
        /* .... the coef function transformation
          .... from reference coordinates to original coordinates */
        shapc(nrefc,ncoor,1,coefd,coefc,crtr,2,5,5);
        weigh=det*gaus[igaus];
        vol = 1.0;
        curlazx=+coefc[(1-1)*(5)+2-1];
        curlazy=-coefc[(1-1)*(5)+1-1];
// .... the following is the stiffness computation
        for (i=1; i<=4; ++i)
        {
            iv=kvord[(i-1)*(2)+1-1];
            for (j=1; j<=4; ++j)
            {
                jv=kvord[(j-1)*(2)+1-1];
                stif=+chx[(i-1)*(3)+1-1]*chx[(j-1)*(3)+1-1]*0.0;
                estif[(iv-1)*(8)+jv-1]+=stif*weigh;
            }
        }
// .... the following is the mass matrix computation
        stif=vol;
        elump[1]=stif*weigh;
        stif=vol;
        elump[3]=stif*weigh;
        stif=vol;
        elump[5]=stif*weigh;
        stif=vol;
        elump[7]=stif*weigh;
        stif=vol;
        elump[2]=stif*weigh;
        stif=vol;
        elump[4]=stif*weigh;
        stif=vol;
        elump[6]=stif*weigh;
        stif=vol;
        elump[8]=stif*weigh;
        for (i=1; i<=nvard[1]; ++i)
        {
            iv = kvord[(i-1)*(2)+1-1];
            emass[iv]+=elump[iv]*chx[(i-1)*(3)+1-1];
        }
        for (i=1; i<=nvard[2]; ++i)
        {
            iv = kvord[(i-1)*(2)+2-1];
            emass[iv]+=elump[iv]*chy[(i-1)*(3)+1-1];
        }
// .... the following is the load vector computation
        for (i=1; i<=4; ++i)
        {
            iv=kvord[(i-1)*(2)+1-1];
            stif=+chx[(i-1)*(3)+1-1]*vol/fmu*curlazx;
            eload[iv]+=stif*weigh;
        }
        for (i=1; i<=4; ++i)
        {
            iv=kvord[(i-1)*(2)+2-1];
            stif=+chy[(i-1)*(3)+1-1]*vol/fmu*curlazy;
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
    ngaus=  4;
    ndisp=  2;
    nrefc=  2;
    ncoor=  2;
    nvar =  8;
    nnode=  4;
    kdord[1]=1;
    nvard[1]=4;
    kvord[(1-1)*(2)+1-1]=1;
    kvord[(2-1)*(2)+1-1]=3;
    kvord[(3-1)*(2)+1-1]=5;
    kvord[(4-1)*(2)+1-1]=7;
    kdord[2]=1;
    nvard[2]=4;
    kvord[(1-1)*(2)+2-1]=2;
    kvord[(2-1)*(2)+2-1]=4;
    kvord[(3-1)*(2)+2-1]=6;
    kvord[(4-1)*(2)+2-1]=8;
    refc[(1-1)*(4)+1-1]=5.773502692e-001;
    refc[(2-1)*(4)+1-1]=5.773502692e-001;
    gaus[1]=1.000000000e+000;
    refc[(1-1)*(4)+2-1]=5.773502692e-001;
    refc[(2-1)*(4)+2-1]=-5.773502692e-001;
    gaus[2]=1.000000000e+000;
    refc[(1-1)*(4)+3-1]=-5.773502692e-001;
    refc[(2-1)*(4)+3-1]=5.773502692e-001;
    gaus[3]=1.000000000e+000;
    refc[(1-1)*(4)+4-1]=-5.773502692e-001;
    refc[(2-1)*(4)+4-1]=-5.773502692e-001;
    gaus[4]=1.000000000e+000;
    return;
}


// void dshap(double (*shap)(double *,double *,int),double *,^shpr[][4],int,int,int);
static void shap_hx(refc,shpr)
double *refc,shpr[12];
/* compute shape functions and their partial derivatives
 shapr ---- store shape functions and their partial derivatives */
{
    double (*shap)(double *,int)=&fshap_hx;
// extern void dshap(shap,double *,double *,int,int,int);
    dshap(shap,refc,shpr,2,4,1);
    /* shape function and their derivatives computation
     compute partial derivatives by centered difference
     which is in the file cshap.c of felac library */
    return;
}

static double fshap_hx(double *refc,int n)
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
        fval=+(+1.-rx)/2.*(+1.-ry)/2.;
        break;
    case 2:
        fval=+(+1.+rx)/2.*(+1.-ry)/2.;
        break;
    case 3:
        fval=+(+1.+rx)/2.*(+1.+ry)/2.;
        break;
    case 4:
        fval=+(+1.-rx)/2.*(+1.+ry)/2.;
        break;
//   default:
    }
    return fval;
}
static void shap_hy(refc,shpr)
double *refc,shpr[12];
/* compute shape functions and their partial derivatives
 shapr ---- store shape functions and their partial derivatives */
{
    double (*shap)(double *,int)=&fshap_hy;
// extern void dshap(shap,double *,double *,int,int,int);
    dshap(shap,refc,shpr,2,4,1);
    /* shape function and their derivatives computation
     compute partial derivatives by centered difference
     which is in the file cshap.c of felac library */
    return;
}

static double fshap_hy(double *refc,int n)
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
        fval=+(+1.-rx)/2.*(+1.-ry)/2.;
        break;
    case 2:
        fval=+(+1.+rx)/2.*(+1.-ry)/2.;
        break;
    case 3:
        fval=+(+1.+rx)/2.*(+1.+ry)/2.;
        break;
    case 4:
        fval=+(+1.-rx)/2.*(+1.+ry)/2.;
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
    double x[5],y[5];
    int j;
    for (j=1; j<=4; ++j)
    {
        x[j]=coorr[(1-1)*(4)+j-1];
        y[j]=coorr[(2-1)*(4)+j-1];
    }
    rx=refc[1];
    ry=refc[2];
    switch (n)
    {
    case 1:
        fval=
            +(+(+1.-rx)/2.*(+1.-ry)/2.)*x[1]
            +(+(+1.+rx)/2.*(+1.-ry)/2.)*x[2]
            +(+(+1.+rx)/2.*(+1.+ry)/2.)*x[3]
            +(+(+1.-rx)/2.*(+1.+ry)/2.)*x[4];
        break;
    case 2:
        fval=
            +(+(+1.-rx)/2.*(+1.-ry)/2.)*y[1]
            +(+(+1.+rx)/2.*(+1.-ry)/2.)*y[2]
            +(+(+1.+rx)/2.*(+1.+ry)/2.)*y[3]
            +(+(+1.-rx)/2.*(+1.+ry)/2.)*y[4];
        break;
        //default:
    }
    return fval;
}

static void coef_shap(double *refc,double *coef,double *coefr,double *coefd)
/* compute coef value and their partial derivatives
by reference coordinate value */
{
    double (*shap)(double *,int)=&fcoef_shap;
// extern void dcoef(shap,double *,double *,double *,
//  int,int,int);
    dcoef(shap,refc,coef,coefd,2,1,2);
    /* coef value and their partial derivatives caculation
     compute partial derivatives by centered difference
     which is in the file cshap.c of felac library */
    return;
}

static double fcoef_shap(double *refc,int n)
/* coef function caculation */
{
    double rx,ry;
    double x,y,fval;
    double az[5];
    int j;
    for (j=1; j<=4; ++j)
    {
        az[j]=coefr[(1-1)*(4)+j-1];
    }
    x=coor[1];
    y=coor[2];
    rx=refc[1];
    ry=refc[2];
    switch (n)
    {
    case 1:
        fval=
            +(+(+1.-rx)/2.*(+1.-ry)/2.)*az[1]
            +(+(+1.+rx)/2.*(+1.-ry)/2.)*az[2]
            +(+(+1.+rx)/2.*(+1.+ry)/2.)*az[3]
            +(+(+1.-rx)/2.*(+1.+ry)/2.)*az[4];
        break;
        //default:
    }
    return fval;
}
