program modulo_test
  implicit none
  integer, parameter :: n=1024
  real :: array(n,n)
  integer :: i
  !$omp declare target map(alloc:array)

  !$omp target teams distribute parallel do simd
  do i=1,size(array)
    array(modulo(i,n),i/n)=i
  enddo
end program modulo_test