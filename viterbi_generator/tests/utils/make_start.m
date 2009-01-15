function [ START ] = make_start(Lh,SYMBOLS)
%
% MAKE_START:
%           This code returns a statenumber corresponding to the start
%           state as it is found from Lh. The method is to use the table
%           of symbolic start states as it is listed in the report made 
%           by 95gr870T. For the table lookups are made in SYMBOLS. in 
%           order to map from the symbol representation to the state number 
%           representation.
%
% SYNTAX:   [ START ] = make_start(Lh,SYMBOLS)
%
% INPUT:    SYMBOLS: The table of symbols corresponding the the state-
%                    numbers.
%           Lh:      Length of the estimated impulseresponse.
%
% OUTPUT:   START:   The number representation of the legal start state.
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
% $Id: make_start.m,v 1.2 1997/09/22 11:40:17 aneks Exp $

% WE HAVEN'T FOUND IT YET.
%
START_NOT_FOUND = 1;

% OBTAIN THE SYMBOLS FROM Lh. THIS IS THE TABLE LISTED IN THE REPORT MADE 
% BY 95gr870T. (SATEREPRESENTATION IS SLIGHTLY CHANGED).
%
if Lh==1,
  start_symbols = [ 1 ];
elseif Lh==2,
  start_symbols = [ 1 -j ];
elseif Lh==3,
  start_symbols = [ 1 -j -1 ];
elseif Lh==4,
  start_symbols = [ 1 -j -1 j ];
elseif Lh==5,
  start_symbols = [ 1 -j -1 j 1];
else
  fprintf('\n\nError: Illegal value of Lh, terminating...');
end

% NOW MAP FROM THE SYMBOLS TO A STATE NUMBER BY SEARCHING 
% SYMBOLS.
%
START=0;
while START_NOT_FOUND,
  START=START+1;
  if sum(SYMBOLS(START,:)==start_symbols)==Lh,
    START_NOT_FOUND=0;
  end
end
