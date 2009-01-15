function [ PREVIOUS ] = make_previous(SYMBOLS)
%
% MAKE_PREVIOUS:       
%           This function returns a lookuptable containing a mapping
%           between the present state and the legal previous states.
%           Each row correspond to a state, and the two legal states
%           related to state n is located in PREVIOUS(n,1) and in
%           previous(n,2). States are represented by their related 
%           numbers.
%
% SYNTAX:   [ PREVIOUS ] = make_previous(SYMBOLS)
%
% INPUT:    SYMBOLS: The table of symbols corresponding the the state-
%                    numbers.
%
% OUTPUT:   PREVIOUS:
%                    The transition table describing the legal previous
%		     states asdescribed above.
%
% SUB_FUNC: None
%
% WARNINGS: None
%
% TEST(S):  Verified against expected result.
%
% AUTOR:    Jan H. Mikkelsen / Arne Norre Ekstrøm
% EMAIL:    hmi@kom.auc.dk / aneks@kom.auc.dk
%
% $Id: make_previous.m,v 1.3 1997/09/22 08:14:27 aneks Exp $

% FIRST WE NEED TO FIND THE NUMBER OF LOOPS WE SHOULD RUN.
% THIS EQUALS THE NUMBER OF SYMBOLS. ALSO MAXSUM IS NEEDED FOR
% LATER OPERATIONS.
%
[ states , maxsum ]=size(SYMBOLS);

maxsum=maxsum-1;
search_matrix=SYMBOLS(:,1:maxsum);

% LOOP OVER THE SYMBOLS.
%
for this_state=1:states,
  search_vector=SYMBOLS(this_state,2:maxsum+1);
  k=0;
  for search=1:states,
    if (sum(search_matrix(search,:)==search_vector)==maxsum)
      k=k+1;
      PREVIOUS(this_state,k)=search;
      if k > 2,
	error('Error: identified too many previous states');
      end
    end
  end
end