#include "projection.hpp"
#include "../predicates/inside_prism_tetra.hpp"

#include "../common.hpp"
#include <Eigen/Dense>
#include <igl/barycentric_coordinates.h>
#include <igl/ray_mesh_intersect.h>
#include <list>
#include <math.h>
#include <spdlog/spdlog.h>

auto canonical_prism =
    std::array<Vec3d, 6>{Vec3d(0, 0, 0), Vec3d(1, 0, 0), Vec3d(0, 1, 0),
                         Vec3d(0, 0, 1), Vec3d(1, 0, 1), Vec3d(0, 1, 1)};
// for one prism, tuple record a,b,t
bool prism::phong::phong_projection(const std::array<Vec3d, 6> &stacked_V,
                                    const Vec3d &point, bool tetra_split_way,
                                    std::array<double, 3> &tuple) {
  auto &tetconfig = tetra_split_way ? TETRA_SPLIT_A : TETRA_SPLIT_B;

  // find where is the point
  for (auto i = 2; i > (stacked_V[0] == stacked_V[3] ? 0 : -1); i--)
    if (predicates::point_in_tetrahedron(
            point, stacked_V[tetconfig[i][0]], stacked_V[tetconfig[i][1]],
            stacked_V[tetconfig[i][2]], stacked_V[tetconfig[i][3]])) {

      Eigen::RowVector4d bary;
      igl::barycentric_coordinates(
          point, stacked_V[tetconfig[i][0]], stacked_V[tetconfig[i][1]],
          stacked_V[tetconfig[i][2]], stacked_V[tetconfig[i][3]], bary);
      if (bary.hasNaN()) {
        spdlog::error("Nan Bary");
        exit(1);
      };
      Vec3d sol = Vec3d(0, 0, 0);
      for (int j = 0; j < 4; j++) {
        sol += bary[j] * canonical_prism[tetconfig[i][j]];
      }
      tuple[0] = sol[0];
      tuple[1] = sol[1];
      tuple[2] = sol[2];
      if (std::isnan(tuple[0]) || std::isnan(tuple[1])) {
        spdlog::error("tup Bary");
        exit(1);
      };
      return true;
    }
  return false;
}

void prism::phong::fiber_endpoints(const std::array<Vec3d, 6> &stacked_V,
                                   bool tetra_split_way, double u, double v,
                                   std::array<Vec3d, 4> &endpoints) {
  auto &rfconfig =
      tetra_split_way ? PRISM_RISING_FACETS_A : PRISM_RISING_FACETS_B;
  for (int i = 0; i < 4; i++) {
    endpoints[i] = stacked_V[rfconfig[i][0]] * (1 - u - v) +
                   stacked_V[rfconfig[i][1]] * u +
                   stacked_V[rfconfig[i][2]] * v;
  }
}
