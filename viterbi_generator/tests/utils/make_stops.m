function [ STOPS ] = make_stops(Lh,SYMBOLS)
%
% MAKE_STOPS:
%           This code returns a statenumber corresponding to the set of
%           legal stop states as found from Lh. The method is to use the table
%           of symbolic stop states as it is listed in the report made 
%           by 95gr870T. For the table lookups are made in SYMBOLS. in 
%           order to map from the symbol representation to the state number 
%           representation.
%
% SYNTAX:   [ STOPS ] = make_stops(Lh,SYMBOLS)
%
% INPUT:    SYMBOLS: The table of symbols corresponding the the state-
%                    numbers.
%           Lh:      Length of the estimated impulseresponse.
%
% OUTPUT:   STOPS:   The number representation of the set of legal stop 
%                    states.
%
% SUB_FUNC: None
%
% WARNINGS: The table of symbolic representations has not been verified
%           but is used directly as it is listed in the report made 
%           by 95gr870T.
%
% TEST(S):  The function has been verified to return a state number
%           which matches the symbolic representation.
%
% AUTOR:    Jan H. Mikkelsen / Arne Norre Ekstrøm
% EMAIL:    hmi@kom.auc.dk / aneks@kom.auc.dk
%
% $Id: make_stops.m,v 1.2 1997/09/22 11:44:21 aneks Exp $

% OBTAIN THE SYMBOLS FROM Lh. THIS IS THE TABLE LISTED IN THE REPORT MADE 
% BY 95gr870T. (SATEREPRESENTATION IS SLIGHTLY CHANGED).
%
if Lh==1,
  stop_symbols = [ -1 ];
  count=1;
elseif Lh==2,
  stop_symbols = [ -1 j ];
  count=1;
elseif Lh==3,
  stop_symbols = [ -1 j 1 ];
  count=1;
elseif Lh==4,
  stop_symbols = [  [ -1 j 1 j ] ; [ -1 j 1 -j ] ];
  count=2;
elseif Lh==5,
  stop_symbols = [ [ -1 j 1 j -1] ; [ -1 j 1 -j -1] ; [ -1 j 1 j 1] ; [ -1 j 1 -j 1] ];
  count=2;
else
  fprintf('\n\nError: Illegal value of Lh, terminating...');
end

% NOW THAT WE HAVE THE SYMBOL REPRESENTATION THE REMAINING JOB IS
% TO MAP THE MSK SYMBOLS TO STATE NUMBERS
%
index = 0;
stops_found=0;
while stops_found < count,
  index=index+1;
  if sum(SYMBOLS(index,:)==stop_symbols(stops_found+1,:))==Lh,
    stops_found=stops_found+1;
    STOPS(stops_found)=index;
  end
end
