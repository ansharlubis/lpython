program dr_fortran
implicit none

   character(len = 5) :: intro
   character(len = 8) :: verb
   character(len = 5) :: posit
   character(len = 4) :: title
   character(len = 7) :: last_name
   character(len = 29) :: combined
   
   intro = "I've "
   verb = "learned "
   posit = "from "
   title = 'Dr. ' 
   last_name = 'Fortran'
   
   combined = intro//verb//posit//title//last_name
   
   print *, combined
   
end program dr_fortran
