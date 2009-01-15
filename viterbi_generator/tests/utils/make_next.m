function [ NEXT ] = make_next(SYMBOLS)
%
% MAKE_NEXT:
%           This function returns a lookuptable containing a mapping
%           between the present state and the legal next states.
%           Each row correspond to a state, and the two legal states
%           related to state n is located in NEXT(n,1) and in
%           NEXT(n,2). States are represented by their related 
%           numbers.
%
% SYNTAX:   [ NEXT ] = make_next(SYMBOLS)
%
% INPUT:    SYMBOLS: The table of symbols corresponding the the state-
%                    numbers.
%
% OUTPUT:   NEXT:    The transition table describing the legal next
%		     states asdescribed above.
%
% SUB_FUNC: None
%
% WARNINGS: None
%
% TEST(S):  The function has been verified to return the expected
%           results.
%
% AUTOR:    Jan H. Mikkelsen / Arne Norre Ekstrøm
% EMAIL:    hmi@kom.auc.dk / aneks@kom.auc.dk
%
% $Id: make_next.m,v 1.3 1997/09/22 08:13:29 aneks Exp $

% FIRST WE NEED TO FIND THE NUMBER OF LOOPS WE SHOULD RUN.
% THIS EQUALS THE NUMBER OF SYMBOLS. ALSO MAXSUM IS NEEDED FOR
% LATER OPERATIONS.
%
[ states , maxsum ]=size(SYMBOLS);

search_matrix=SYMBOLS(:,2:maxsum);
maxsum=maxsum-1;

% LOOP OVER THE SYMBOLS.
%
for this_state=1:states,
  search_vector=SYMBOLS(this_state,1:maxsum);
  k=0;
  for search=1:states,
    if (sum(search_matrix(search,:)==search_vector)==maxsum)
      k=k+1;
      NEXT(this_state,k)=search;
      if k > 2,
	error('Error: identified too many next states');
      end
    end
  end
end