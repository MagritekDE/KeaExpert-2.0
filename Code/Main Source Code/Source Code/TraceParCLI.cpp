#include "stdafx.h"
#include "TraceParCLI.h"
#include "cArg.h"
#include "ctext.h"
#include "trace.h"
#include "error.h"
#include "evaluate_simple.h"
#include "globals.h"
#include "interface.h"
#include "mymath.h"
#include "plot1dCLI.h"
#include "plot.h"
#include "TracePar.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include <vector>
#include <string>
#include <utility>
#include <typeinfo>
#include "memoryLeak.h"

using std::vector;
using std::string;
using std::pair;

/************************************************************************
* TraceParCLI
*
* Command line access to TracePar
*
* (c) Magritek 2011
*************************************************************************/

short GetOrSetTraceParameters(Interface *itfc, CArg *carg, short nrArgs, short start, Trace *di, TracePar& tracePar, Plot1D *plot)
{
	CText parameter;
	CText value;
	short i;
	short type;
	Variable result;
	bool extractArg = false;

	// Check for valid number of arguments
	if(nrArgs == 1)
	{
		extractArg = true;
		nrArgs = 2;
	}
	else if((nrArgs-start+1)%2 != 0)
	{
		ErrorMessage("number of arguments must be even");
		return(ERR);
	}

	// Loop over parameter pairs *************************************      
	i = start;
	while(i < nrArgs)
	{
		// Extract parameter name
		parameter = carg->Extract(i);
		if((type = Evaluate(itfc,RESPECT_ALIAS,parameter.Str(),&result)) < 0)
			return(ERR); 

		if(type != UNQUOTED_STRING)
		{
			ErrorMessage("invalid data type for parameter '%s'",carg->Extract(i));
			return(ERR);
		}  
		parameter = result.GetString();

		// Extract trace parameter value
		value = carg->Extract(i+1);

		// Get trace parameter ****************************************************

		if(extractArg || value == "" || value == "\"\"" || value == "\"getargs\"" || value == "\"getarg\"")
		{
			if(parameter == "getdata" && di)
			{
				Variable* xAsVar = di->xComponentAsVariable();
				Variable* yAsVar = di->yComponentAsVariable();
				itfc->retVar[1].Assign(xAsVar);
				itfc->retVar[2].Assign(yAsVar);
				itfc->nrRetValues = 2;
				return(OK);
			}
			else if (parameter == "size" && di)
			{
				int sz = di->getSize();
				itfc->retVar[1].MakeAndSetFloat(sz);
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "parent" && di)
			{
				Plot *pd = di->getParent();
				itfc->retVar[1].MakeClass(PLOT_CLASS,(void*)pd);
				itfc->nrRetValues = 1;
				return(OK);
			}                 
			else if(parameter == "id" && di)
			{
				itfc->retVar[1].MakeAndSetFloat(di->getID());
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "tracecolor" || parameter == "color")
			{
				COLORREF c = tracePar.getRealColor();
				BYTE *data = (BYTE*)&c;
				float colors[6];
				colors[0] = data[0];
				colors[1] = data[1];
				colors[2] = data[2];
				c = tracePar.getImagColor();
				data = (BYTE*)&c;
				colors[3] = data[0];
				colors[4] = data[1];
				colors[5] = data[2];
				itfc->retVar[1].MakeMatrix2DFromVector(colors,6,1);
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "realcolor")
			{
				COLORREF c = tracePar.getRealColor();
				BYTE *data = (BYTE*)&c;
				float colors[3];
				colors[0] = data[0];
				colors[1] = data[1];
				colors[2] = data[2];
				itfc->retVar[1].MakeMatrix2DFromVector(colors,3,1);
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "imagcolor")
			{
				COLORREF c = tracePar.getImagColor();
				BYTE *data = (BYTE*)&c;
				float colors[3];
				colors[0] = data[0];
				colors[1] = data[1];
				colors[2] = data[2];
				itfc->retVar[1].MakeMatrix2DFromVector(colors,3,1);
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "tracewidth")
			{
            float width = tracePar.getTraceWidth()/2.0+0.5; // Internal width rep is 1,2,3,4 ...
            if(width < 1.0) width = 1.0;
				itfc->retVar[1].MakeAndSetFloat(width); // External width rep is 1, 1.5, 2.0, 2.5, ...
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "symbolcolor" || parameter == "realsymbolcolor")
			{
				COLORREF c = tracePar.getRealSymbolColor();
				BYTE *data = (BYTE*)&c;
				float colors[3];
				colors[0] = data[0];
				colors[1] = data[1];
				colors[2] = data[2];
				itfc->retVar[1].MakeMatrix2DFromVector(colors,3,1);
				itfc->nrRetValues = 1;
				return(OK);
			} 
			else if(parameter == "imagsymbolcolor")
			{
				COLORREF c = tracePar.getImagSymbolColor();
				BYTE *data = (BYTE*)&c;
				float colors[3];
				colors[0] = data[0];
				colors[1] = data[1];
				colors[2] = data[2];
				itfc->retVar[1].MakeMatrix2DFromVector(colors,3,1);
				itfc->nrRetValues = 1;
				return(OK);
			}  
		   else if(parameter == "whitewash")
		   {
				itfc->retVar[1].MakeAndSetFloat(tracePar.getWhiteWash());
				itfc->nrRetValues = 1;
				return(OK);
		   } 
		   else if(parameter == "xoffset")
		   {
				itfc->retVar[1].MakeAndSetFloat(tracePar.getXOffset());
				itfc->nrRetValues = 1;
				return(OK);
		   } 
		   else if(parameter == "yoffset")
		   {
				itfc->retVar[1].MakeAndSetFloat(tracePar.getYOffset());
				itfc->nrRetValues = 1;
				return(OK);
		   } 
			else if(parameter == "symbolsize")
			{
				itfc->retVar[1].MakeAndSetFloat(tracePar.getSymbolSize());
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "symbolshape")
			{   
				char* c = strdup(TracePar::GetSymbolTypeStr(tracePar.getSymbolType()));
            if(c)
            {
				   itfc->retVar[1].MakeAndSetString(c);
				   itfc->nrRetValues = 1;
				   free(c);
            }
				return(OK);
			}
			else if(parameter == "tracetype")
			{   
				char* c = strdup(TracePar::GetTraceTypeStr(tracePar.getTraceType()));
            if(c)
            {
				   itfc->retVar[1].MakeAndSetString(c);
				   itfc->nrRetValues = 1;
				   free(c);
            }
				return(OK);
			}
			else if(parameter == "realstyle")
			{  
				itfc->retVar[1].MakeAndSetFloat(tracePar.getRealStyle()); 
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "imagstyle")
			{  
				itfc->retVar[1].MakeAndSetFloat(tracePar.getImagStyle()); 
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "ebarcolor" || parameter == "ebar_color")
			{   
				COLORREF c = tracePar.getBarColor();
				BYTE *data = (BYTE*)&c;
				float colors[3];
				colors[0] = data[0];
				colors[1] = data[1];
				colors[2] = data[2];
				itfc->retVar[1].MakeMatrix2DFromVector(colors,3,1);
				itfc->nrRetValues = 1;
				return(OK);
			}
			else if(parameter == "ebarsize" || parameter == "ebar_size")
			{
				itfc->retVar[1].MakeAndSetFloat(tracePar.getBarFixedHeight());
				itfc->nrRetValues = 1;
				return(OK);

			} 
			else if(parameter == "ebarfixed" || parameter == "ebar_fixed")
			{
				if(tracePar.isFixedErrorBars())
					itfc->retVar[1].MakeAndSetString("true");
				else
					itfc->retVar[1].MakeAndSetString("false");
				itfc->nrRetValues = 1;
				return(OK);
			}  
			else if(parameter == "ebarshow" || parameter == "ebar_show")
			{
				if(tracePar.isShowErrorBars())
					itfc->retVar[1].MakeAndSetString("true");
				else
					itfc->retVar[1].MakeAndSetString("false");
				itfc->nrRetValues = 1;
				return(OK);
			} 
			else if((parameter == "ebararray" || parameter == "ebar_array") && di)
			{   
				if(di->HasBars())
				{
					Variable* barsAsVar = di->barsAsVariable();
					itfc->retVar[1].FullCopy(barsAsVar);
					itfc->nrRetValues = 1;
					delete barsAsVar;
					return(OK);
				}
				else
				{
					itfc->retVar[1].MakeNullVar();
					itfc->nrRetValues = 1;
					return(OK);
				}              
			}
		   else if (parameter == "axis")
		   {
			   if(type != UNQUOTED_STRING)
			   {
				   ErrorMessage("invalid data type for axis parameter");
				   return(ERR);
			   } 
				if(di->yAxis()->side() == RIGHT_VAXIS_SIDE)
					itfc->retVar[1].MakeAndSetString("right");
				else
					itfc->retVar[1].MakeAndSetString("left");
				itfc->nrRetValues = 1;
				return(OK);
		   }
		   else if (parameter == "name")
		   {
			   if(type != UNQUOTED_STRING)
			   {
				   ErrorMessage("invalid data type for axis parameter");
				   return(ERR);
			   } 
            if(di->getName())
			      itfc->retVar[1].MakeAndSetString((char*)di->getName());
            else
               itfc->retVar[1].MakeAndSetString("");
				itfc->nrRetValues = 1;
				return(OK);
		   }
			else if(parameter == "inlegend")
			{
				if(di->appearsInLegend())
					itfc->retVar[1].MakeAndSetString("true");
				else
					itfc->retVar[1].MakeAndSetString("false");
				itfc->nrRetValues = 1;
				return(OK);
			} 
			else if(parameter == "ignorexrange")
			{
				if(di->getIgnoreXRange())
					itfc->retVar[1].MakeAndSetString("true");
				else
					itfc->retVar[1].MakeAndSetString("false");
				itfc->nrRetValues = 1;
				return(OK);
			} 
			else if(parameter == "ignoreyrange")
			{
				if(di->getIgnoreYRange())
					itfc->retVar[1].MakeAndSetString("true");
				else
					itfc->retVar[1].MakeAndSetString("false");
				itfc->nrRetValues = 1;
				return(OK);
			} 
         else if(parameter == "uservar")
         {
            Variable *var = di->varList.next;
            if(var)
               itfc->retVar[1].FullCopy(var);
            else
               itfc->retVar[1].MakeNullVar();
				itfc->nrRetValues = 1;
				return(OK);
         }
			else
			{
				ErrorMessage("Invalid or unknown trace parameter '%s'",parameter.Str());
				return(ERR);
			}
		}


		// Set trace parameter value ****************************************
		if((type = Evaluate(itfc,RESPECT_ALIAS,value.Str(),&result)) < 0)
			return(ERR);

		itfc->nrRetValues = 0; 
		value.RemoveQuotes();

		// Process different kinds of parameter   
      if(parameter == "uservar")
      {
		   if(type == STRUCTURE)
         {
            CText par;
            di->varList.RemoveAll();
            Variable *dstVar = di->varList.Add(STRUCTURE,"objStruct");
            Variable *srcVar = &result;
            srcVar->SetType(STRUCTURE);
            CopyVariable(dstVar,srcVar,FULL_COPY);
         }
		   else
         {
            ErrorMessage("User variable should be a parameter string");
            return(ERR);
         }
      }
		else if (parameter == "setdata")
		{
			bool err = true;
			if (type == STRUCTURE)
			{
				Variable* xVar,*yVar;

				xVar = result.GetStruct()->next;
				if (xVar)
				{
					yVar = xVar->next;
					if (yVar)
					{
						string diType = di->typeString();

						if ((diType == "Real") && (xVar->GetType() == MATRIX2D && yVar->GetType() == MATRIX2D) && (xVar->GetDimX() == yVar->GetDimX()) && (xVar->GetDimY() == 1 && yVar->GetDimY() == 1))
						{
							long size = xVar->GetDimX();
							float* xData = CopyArray(xVar->GetMatrix2D()[0], size);
							float* yData = CopyArray(yVar->GetMatrix2D()[0], size);
						   dynamic_cast<TraceReal*>(di)->setData(xData, yData, size);
							err = false;
						}
						else if ((diType == "Complex") && (xVar->GetType() == MATRIX2D && yVar->GetType() == CMATRIX2D) && (xVar->GetDimX() == yVar->GetDimX()) && (xVar->GetDimY() == 1 && yVar->GetDimY() == 1))
						{
							long size = xVar->GetDimX();
							float* xData = CopyArray(xVar->GetMatrix2D()[0], size);
							complex* yData = CopyCArray(yVar->GetCMatrix2D()[0], size);
							dynamic_cast<TraceComplex*>(di)->setData(xData, yData, size);
							err = false;
						}
						else
						{
							ErrorMessage("invalid data type for xData or yData (should be real x and real/complex y vectors of same length and matching existing trace type)");
							return(ERR);
						}
					}
				}
			}
			//else if (type == CMATRIX2D && result.GetDimY() == 2)
			//{
			//	complex* xData = result.GetCMatrix2D()[0];
			//	complex* yData = result.GetCMatrix2D()[1];
			//	long size = result.GetDimX();
			//	char traceName[10] = "";
			//	dynamic_cast<TraceComplex*>(di)->setData(xData, yData, size);
			//	result.SetNull();
			//}
			if(err)
			{
				ErrorMessage("invalid data for setdata (structure with x and y vector components)");
				return(ERR);
			}
		}
		else if(parameter == "symbolshape")
		{   
			if(type != UNQUOTED_STRING)
			{
				ErrorMessage("invalid data type for symbolshape (string)");
				return(ERR);
			} 
			value = result.GetString();
			int symbol = TracePar::GetSymbolTypeCode(value.Str());
			if (symbol < 0)
			{
				ErrorMessage("invalid value '%s'",value.Str());
				return(ERR);
			}
			tracePar.setSymbolType(symbol);
		}
		else if(parameter == "tracetype")
		{
			if(type != UNQUOTED_STRING)
			{
				ErrorMessage("invalid data type for tracetype (string)");
				return(ERR);
			} 
			value = result.GetString();
			int ttype = TracePar::GetTraceTypeCode(value.Str());
			if (ttype < 0)
			{
				ErrorMessage("invalid value '%s'",value.Str());
				return(ERR);
			}  
			tracePar.setTraceType(ttype);
		}
		else if(parameter == "tracecolor" || parameter == "color")
		{
			COLORREF c,bc=0;
			if(SetColor(c,&result) == ERR)
				return(ERR);
			tracePar.setRealColor(c); // Set imag color midway between real and background
         if(di)
            bc = di->getParent()->bkColor;
         else if(plot)
            bc = plot->bkColor;

         if(bc)
            tracePar.setImagColor(RGB( (GetRValue(bc)+GetRValue(c))/2,
                                       (GetGValue(bc)+GetGValue(c))/2,
                                       (GetBValue(bc)+GetBValue(c))/2));
		}
		else if(parameter == "realcolor")
		{
			COLORREF c;
			if(SetColor(c,&result) == ERR)
				return(ERR);
			tracePar.setRealColor(c);
		}
		else if(parameter == "imagcolor")
		{
			COLORREF c;
			if(SetColor(c,&result) == ERR)
				return(ERR);
			tracePar.setImagColor(c);
		}
		else if(parameter == "ignorexrange")
		{
			if(type != UNQUOTED_STRING)
			{
				ErrorMessage("invalid data type for ignorexRange");
				return(ERR);
			} 
			value = result.GetString();

			if(value == "true")
				di->setIgnoreXRange(true);
			else if(value == "false")
				di->setIgnoreXRange(false);
			else
			{
				ErrorMessage("invalid value '%s'",value.Str());
				return(ERR);
			}
		}
		else if(parameter == "ignoreyrange")
		{
			if(type != UNQUOTED_STRING)
			{
				ErrorMessage("invalid data type for ignoreyRange");
				return(ERR);
			} 
			value = result.GetString();

			if(value == "true")
				di->setIgnoreYRange(true);
			else if(value == "false")
				di->setIgnoreYRange(false);
			else
			{
				ErrorMessage("invalid value '%s'",value.Str());
				return(ERR);
			}
		}
		else if(parameter == "tracewidth")
		{
			float width;
			if(type != FLOAT32)
			{
				ErrorMessage("invalid type for trace width (float)");
				return(ERR);
			}

			width = result.GetReal(); // External width rep is 1, 1.5, 2.0, 2.5, ...

			if(width < 1 || width > 20)
			{
				ErrorMessage("invalid trace width (valid range is 1 to 20)");
				return(ERR);
			}
			tracePar.setTraceWidth((width-0.5)*2.0); // Internal width rep is 1,2,3,4 ... 
		}
		else if(parameter == "linestyle" || parameter == "realstyle")
		{
			short style;
			if(type != FLOAT32)
			{
				ErrorMessage("invalid type for trace style (float)");
				return(ERR);
			}

			style = nint(result.GetReal());

			if(style < 0 || style > 4)
			{
				ErrorMessage("invalid line style (valid range is 0 to 4)");
				return(ERR);
			}
			tracePar.setRealStyle(style);  
		}
		else if(parameter == "imagstyle")
		{
			short style;
			if(type != FLOAT32)
			{
				ErrorMessage("invalid type trace width (float)");
				return(ERR);
			}

			style = nint(result.GetReal());

			if(style < 0 || style > 4)
			{
				ErrorMessage("invalid line style (valid range is 0 to 4)");
				return(ERR);
			}
			tracePar.setImagStyle(style);  
		}
		else if(parameter == "inlegend")
		{
			if(type != UNQUOTED_STRING)
			{
				ErrorMessage("invalid data type for inlegend");
				return(ERR);
			} 
			value = result.GetString();

			if(value == "true")
				tracePar.setInLegend(true);
			else if(value == "false")
				tracePar.setInLegend(false);
			else
			{
				ErrorMessage("invalid value '%s'",value.Str());
				return(ERR);
			}
		}
		else if(parameter == "xoffset")
		{
			float offset;
			if(type != FLOAT32)
			{
				ErrorMessage("invalid xoffset (should be float)");
				return(ERR);
			}
			offset = result.GetReal();
			tracePar.setXOffset(offset);
		} 
		else if(parameter == "yoffset")
		{
			float offset;
			if(type != FLOAT32)
			{
				ErrorMessage("invalid yoffset (should be float)");
				return(ERR);
			}
			offset = result.GetReal();
			tracePar.setYOffset(offset);
		} 
		else if(parameter == "whitewash")
		{
			float offset;
			if(type != FLOAT32)
			{
				ErrorMessage("invalid whitewash state (should be 0/1)");
				return(ERR);
			}
			offset = result.GetReal();
			tracePar.setWhiteWash((bool)offset);
		}
		else if(parameter == "symbolsize")
		{
			short ssize;
			if(type != FLOAT32)
			{
				ErrorMessage("invalid type symbol size (float)");
				return(ERR);
			}

			ssize = nint(result.GetReal());

			if(ssize < 1 || ssize > 20)
			{
				ErrorMessage("invalid symbol size (valid range is 0 to 19)");
				return(ERR);
			}

			tracePar.setSymbolSize(ssize);
		} 
		else if(parameter == "symbolcolor")
		{
			COLORREF c;
			COLORREF bc = 0;
			if(SetColor(c,&result) == ERR)
				return(ERR);    
			tracePar.setRealSymbolColor(c);
         if(di)
            bc = di->getParent()->bkColor;
         else if(plot)
            bc = plot->bkColor;

         if(bc)
            tracePar.setImagSymbolColor(RGB( (GetRValue(bc)+GetRValue(c))/2,
                                       (GetGValue(bc)+GetGValue(c))/2,
                                       (GetBValue(bc)+GetBValue(c))/2));
		}
		else if(parameter == "realsymbolcolor")
		{
			COLORREF c;
			if(SetColor(c,&result) == ERR)
				return(ERR);    
			tracePar.setRealSymbolColor(c);
		}
		else if(parameter == "imagsymbolcolor")
		{
			COLORREF c;
			if(SetColor(c,&result) == ERR)
				return(ERR);    
			tracePar.setImagSymbolColor(c);
		}
		else if((parameter == "ebararray" || parameter == "ebar_array"))
		{   
			if(di)
			{
				long width = result.GetDimX();
				long height = result.GetDimY(); 
				Plot1D *pd = dynamic_cast<Plot1D*>(di->getParent());

				if(result.GetType() != MATRIX2D || width <= 0 || width != pd->curTrace()->getSize() || height != 2)
				{
					ErrorMessage("invalid errorbar matrix");
					return(ERR);
				}

				pd->curTrace()->setBars(CopyMatrix(result.GetMatrix2D(),width,height));
				tracePar.setShowErrorBars(true);
				tracePar.setErrorBarsStored(true);
				tracePar.setFixedErrorBars(false);
			}
		} 
		else if(parameter == "ebarcolor" || parameter == "ebar_color")
		{  
			COLORREF c;
			if(SetColor(c,&result) == ERR)
				return(ERR);  
			tracePar.setBarColor(c);
		}
		else if(parameter == "ebarsize" || parameter == "ebar_size")
		{
			if(type != FLOAT32)
			{
				ErrorMessage("invalid type error bar size (float)");
				return(ERR);
			}        
			tracePar.setBarFixedHeight(result.GetReal());
			tracePar.setShowErrorBars(true);
			tracePar.setFixedErrorBars(true);                            
		} 
		else if(parameter == "ebarfixed" || parameter == "ebar_fixed")
		{
			if(type != UNQUOTED_STRING)
			{
				ErrorMessage("invalid data type for error bar fixed mode (string)");
				return(ERR);
			} 
			value = result.GetString();

			if(value == "true")
				tracePar.setFixedErrorBars(true);
			else if(value == "false")
				tracePar.setFixedErrorBars(false);
			else
			{
				ErrorMessage("invalid value '%s'",value.Str());
				return(ERR);
			}
		}     
		else if(parameter == "ebarshow" || parameter == "ebar_show")
		{
			if(type != UNQUOTED_STRING)
			{
				ErrorMessage("invalid data type for error bar display mode (string)");
				return(ERR);
			} 
			value = result.GetString();

			if(value == "true")
				tracePar.setShowErrorBars(true);
			else if(value == "false")
				tracePar.setShowErrorBars(false);
			else
			{
				ErrorMessage("invalid value '%s'",value.Str());
				return(ERR);
			}
		}   

		else if (parameter == "axis")
		{
			if(type != UNQUOTED_STRING)
			{
				ErrorMessage("invalid data type for axis assignment (string)");
				return(ERR);
			} 
			value = result.GetString();
			if (value == "left")
			{
				if (!di->moveTraceToAxis(LEFT_VAXIS_SIDE))
				{
					ErrorMessage("Trace has y values <= 0 and cannot be displayed on log axis.");
					return ERR;
				}
			}
			else if (value == "right")
			{
				if (!di->moveTraceToAxis(RIGHT_VAXIS_SIDE))
				{
					ErrorMessage("Trace has y values <= 0 and cannot be displayed on log axis.");
					return ERR;
				}
			}
			else
			{
				ErrorMessage("invalid value '%s'",value.Str());
				return(ERR);
			}
			di->getParent()->Invalidate();
		}
		else if (parameter == "name")
		{
		   if(type != UNQUOTED_STRING)
		   {
			   ErrorMessage("invalid data type for trace name parameter (string)");
			   return(ERR);
		   } 
         di->setName(result.GetString());
	   }
		else
		{
			ErrorMessage("invalid parameter '%s'",parameter.Str());
			return(ERR);
		}
		i+=2;        
	}
	return(OK);
}


