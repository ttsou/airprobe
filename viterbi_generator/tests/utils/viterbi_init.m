function [ SYMBOLS , PREVIOUS , NEXT , START , STOPS ] = viterbi_init(Lh)
% VITERBI_INIT:
%           This function returns the tables which are used by the 
%           viterbi demodulator which is implemented in the GSMsim
%           package.
%
% SYNTAX:   [ SYMBOLS , PREVIOUS , NEXT , START , STOPS ]
%           = 
%           viterbi_init(Lh)
%
% INPUT:    Lh:       The length of the channel impulse response
%                     minus one.
%
% OUTPUT:   SYMBOLS:  Statenumber to MSK-symbols mapping table.
%           PREVIOUS: This state to legal previous state mapping table.
%           NEXT:     This state to legal next state mapping table.
%           START:    The start state of the viterbi algorithm.
%           STOPS:    The set of legal stop states for the viterbi 
%                     algorithm.
%
% GLOBAL:   None
%
% SUB_FUNC: make_symbols,make_previous,make_next,make_start,make_stops
%
% WARNINGS: None
% 
% TEST:     Verified that the function actually runs the subfunctions.
%
% AUTHOR:   Jan H. Mikkelsen / Arne Norre Ekstrøm
% EMAIL:    hmi@kom.auc.dk / aneks@kom.auc.dk
%
% $Id: viterbi_init.m,v 1.4 1998/02/12 10:52:15 aneks Exp $

SYMBOLS = make_symbols(Lh);
PREVIOUS = make_previous(SYMBOLS);
NEXT = make_next(SYMBOLS);
START = make_start(Lh,SYMBOLS);
STOPS = make_stops(Lh,SYMBOLS);
