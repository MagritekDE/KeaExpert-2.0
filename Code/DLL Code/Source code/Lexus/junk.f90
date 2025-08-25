SUBROUTINE LexusSQZ(sq, name, cval, clen, rval, status)
USE nrtype  
USE lexus_module
IMPLICIT NONE
! make rnorm and status optional ?
INTEGER(I4B)           :: status
Character(LEN=1)	   :: sq
Character(LEN=4)	   :: name
INTEGER(I4B)           :: clen
Character(LEN=clen)    :: cval
REAL(SP) 	           :: rval 
 ! Local
Character(LEN=1)	   :: sq2
Character(LEN=4)	   :: name2
! 
status=0
sq2=sq
name2=name

IF( sq2 .eq. "S") Then

! Main congtol parameters
  If(name2 .eq. "WEIG" ) Then
    lxWeight=rval
  If(name2 .eq. "EWIT" ) Then
    lxWeight=rval
  ELSEIF(name2 .eq."SIG") Then
    lxSig=rval
  If(name2 .eq. "NALG" ) Then
    lxNAlg=cval
  ELSEIF(name2 .eq."PRLO") Then
    lxPrLow=rval
  ELSEIF(name2 .eq."PRLO") Then
    lxPrLow=rval
  ELSEIF(name2 .eq."PRHI") Then
    lxPrHigh=rval
  ELSEIF(name2 .eq."PRPO") Then
    lxPrPoints=rval
  ELSEIF(name2 .eq."PRAV") Then
    lxPrAv=cval
! advanced control parameters
  ELSEIF(name2 .eq."TOL") Then
    lxTol=rval
  ELSEIF(name2 .eq."MAXI") Then
    lxMaxiter=rval
  ELSEIF(name2 .eq."WITE") Then
    lxWiter =rval
  ELSEIF(name2 .eq."WTOL") Then
    lxWTol =rval
  ELSEIF(name2 .eq."WRAT") Then
    lxWRatio =rval
  ELSEIF(name2 .eq."WFRA") Then
    lxWFrac =rval
  ELSEIF(name2 .eq."RSIG") Then
    lxRSig =rval
  ELSEIF(name2 .eq."C1") Then
    lxC1 =rval
  ELSEIF(name2 .eq."C2") Then
    lxC2 =rval
  ELSEIF(name2 .eq."REPO") Then
    lxWRatio =rval
! Output
  ELSEIF(name2 .eq."CSE") Then
    lxCSE=rval
  ELSEIF(name2 .eq."CWEI") Then
    lxCWEIGHT=rval
  ELSEIF(name2 .eq."CSTA") Then
    lxCSTATUS=rval 
  ELSE
     status=0  
  ENDIF
  
ElseIf( sq2 .eq. "Q") Then

  If(name2 .eq. "WEIG" ) Then
    rval=lxWeight
  ELSE IF(name2 .eq."SIG") Then
    rval=lxSig
  ELSE IF(name2 .eq."MAXI") Then
    rval=lxMaxiter
  ELSE IF(name2 .eq."WITE") Then
    rval=lxWiter
  ELSE IF(name2 .eq."PRLO") Then
    rval=lxPrLow
  ELSE IF(name2 .eq."PRHI") Then
    rval=lxPrHigh
  ELSE IF(name2 .eq."PRPO") Then
    rval=lxPrPoints
  ELSE IF(name2 .eq."CSE") Then
    rval=lxCSE
  ELSE IF(name2 .eq."CWEI") Then
    rval=lxCWEIGHT
  ELSE IF(name2 .eq."CSTA") Then
    rval=lxCSTATUS 
  ELSE IF(name2 .eq. "PRMO") Then
    cval=lxPrmode
  ELSE
    status=1
  END IF
  
  ElseIf( sq2 .eq. "I") Then
     lxWeight=0.01
     lxSig=0.01
     lxMaxiter=50
     lxWiter =40
     lxPrLow=1
     lxPrHigh=-1
     lxPrPoints=200
     lxCSE=0.0
     lxCWEIGHT=0.0
     lxCSTATUS=0.0 
     lxPrMode="A"
Else
  status=1
End if

END SUBROUTINE LexusSQZ