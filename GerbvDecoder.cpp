/*
 * GerbvDecoder.cpp
 *
 *  Created on: Feb 10, 2020
 *      Author: kosa
 */

#include "GerbvDecoder.hpp"
#include <cmath>
#include <memory>

extern "C" {
#include <gerbv.h>
}

using namespace boost::polygon::operators;

static int circle_points = 50;

template<typename coordinate_type>
class rotate_transform {
public:
	explicit rotate_transform(double radians) {
		s = sin(radians);
		c = cos(radians);
	}
	void transform(coordinate_type &x, coordinate_type &y) const {
		coordinate_type tx = x;
		x = x * c - y * s;
		y = tx * s + y * c;
	}
private:
	double s, c;
};

polygon_set makePoly(point center, scalar d, int vertices, double offset) {
	polygon_set pset;
	polygon p;
	std::list<point> nodes;

	double angle_step;
	angle_step = -2 * M_PI / vertices;
	offset *= M_PI / 180.0; // Convert to radians.

	for (int i = 0; i < vertices; i++) {
		nodes.push_back(
				point(int(d / 2 * cos(i * angle_step + offset)) + center.x(),
						int(d / 2 * sin(i * angle_step + offset))
								+ center.y()));
	}
	boost::polygon::set_points(p, nodes.begin(), nodes.end());
	pset += p;
	return pset;
}

polygon_set makePoly(point center, scalar d, int vertices, double offset,
		scalar hole_diameter) {
	polygon_set pset;
	pset += makePoly(center, d, vertices, offset);
	pset -= makePoly(center, hole_diameter, vertices, 0);
	return pset;
}

polygon_set makeRect(point center, scalar width, scalar height,
		scalar hole_diameter, int circle_points) {
	polygon_set pset;
	polygon p;
	std::list<point> nodes;

	int x = center.x();
	int y = center.y();

	nodes.push_back(point(x - width / 2, y - height / 2));
	nodes.push_back(point(x - width / 2, y + height / 2));
	nodes.push_back(point(x + width / 2, y + height / 2));
	nodes.push_back(point(x + width / 2, y - height / 2));

	boost::polygon::set_points(p, nodes.begin(), nodes.end());
	pset += p;
	if (hole_diameter > 0) {
		pset -= makePoly(center, hole_diameter, circle_points, 0);
	}
	return pset;
}

polygon_set makeRect(point point1, point point2, scalar height) {
	polygon_set pset;
	polygon p;
	std::list<point> nodes;
	point angle = point1;
	boost::polygon::deconvolve(angle, point2);
	double scale = boost::polygon::euclidean_distance(point(0, 0), angle);
	boost::polygon::scale_up(angle, height / 2);
	boost::polygon::scale_down(angle, scale);

	nodes.push_back(point(point1.x() - angle.y(), point1.y() + angle.x()));
	nodes.push_back(point(point1.x() + angle.y(), point1.y() - angle.x()));
	nodes.push_back(point(point2.x() + angle.y(), point2.y() - angle.x()));
	nodes.push_back(point(point2.x() - angle.y(), point2.y() + angle.x()));

	boost::polygon::set_points(p, nodes.begin(), nodes.end());
	pset += p;
	return pset;
}

polygon_set makeOval(point center, scalar width, scalar height,
		scalar hole_diameter, int circle_points) {
	polygon_set pset;

	scalar x = center.x();
	scalar y = center.y();

	if (width > height) {
		pset += makePoly(point(x + (width - height) / 2, y), height,
				circle_points, 0);
		pset += makePoly(point(x - (width - height) / 2, y), height,
				circle_points, 0);
		pset += makeRect(center, width - height, height, 0, circle_points);
	} else {
		pset += makePoly(point(x, y + (height - width) / 2), width,
				circle_points, 0);
		pset += makePoly(point(x, y - (height - width) / 2), width,
				circle_points, 0);
		pset += makeRect(center, width, height - width, 0, circle_points);
	}

	if (hole_diameter > 0) {
		pset -= makePoly(center, hole_diameter, circle_points, 0);
	}
	return pset;
}

polygon_set makeMoire(point center, scalar odia, scalar thickness, scalar gap,
		int max_rings, scalar cross_thickness, scalar cross_len) {
	polygon_set pset;
	for (int i = 0; i < max_rings; i++) {
		int external_diameter = odia - 2 * (thickness + gap) * i;
		int internal_diameter = external_diameter - 2 * thickness;
		if (external_diameter <= 0)
			break;
		if (internal_diameter < 0)
			internal_diameter = 0;
		pset += makePoly(center, external_diameter, circle_points, 0,
				internal_diameter);
	}
	pset += makeRect(center, cross_len, cross_thickness, 0, 0);
	pset += makeRect(center, cross_thickness, cross_len, 0, 0);
	return pset;
}

polygon_set makeThermal(point center, scalar odia, scalar idia, scalar gap) {
	polygon_set pset;
	pset += makePoly(center, odia, circle_points, 0, idia);
	pset -= makeRect(center, gap, odia, 0, 0);
	pset -= makeRect(center, odia, gap, 0, 0);
	return pset;
}

polygon_set GerbvDecoder(std::string infile, scalar resolution_in_mm) {
	polygon_set graph;
	gerbv_project_t *project = gerbv_create_project();
	double gscale_to_um;
	gchar *filename = g_strdup(infile.c_str());
	gerbv_open_layer_from_filename(project, filename);
	g_free(filename);

	if (project->file[0] == NULL) {
		std::cerr << "File not found!" << std::endl;
		gerbv_destroy_project(project);
		return graph;
	}

	gerbv_image_t *gerber = project->file[0]->image;
	if (gerber->info->polarity != GERBV_POLARITY_POSITIVE) {
		std::cerr << "Negative polarity not supported" << std::endl;
		gerbv_destroy_project(project);
		return graph;
	}

	if (gerber->netlist->state->unit == GERBV_UNIT_MM) {
		gscale_to_um = resolution_in_mm;
	} else {
		gscale_to_um = resolution_in_mm * 25.4;
	}

	std::map<int, polygon_set> apertures_map;
	{
		gerbv_aperture_t **apertures = gerber->aperture;
		for (int i = 0; i < APERTURE_MAX; i++) {
			const gerbv_aperture_t *const aperture = apertures[i];
			if (!aperture)
				continue;
			switch (aperture->type) {
			case GERBV_APTYPE_NONE:
				continue;
			case GERBV_APTYPE_CIRCLE:
				apertures_map[i] = makePoly(point(0, 0),
						aperture->parameter[0] * gscale_to_um, circle_points,
						aperture->parameter[1],
						aperture->parameter[2] * gscale_to_um);
				break;
			case GERBV_APTYPE_RECTANGLE:
				apertures_map[i] = makeRect(point(0, 0),
						aperture->parameter[0] * gscale_to_um,
						aperture->parameter[1] * gscale_to_um,
						aperture->parameter[2] * gscale_to_um, circle_points);
				break;
			case GERBV_APTYPE_OVAL:
				apertures_map[i] = makeOval(point(0, 0),
						aperture->parameter[0] * gscale_to_um,
						aperture->parameter[1] * gscale_to_um,
						aperture->parameter[2] * gscale_to_um, circle_points);
				break;
			case GERBV_APTYPE_POLYGON:
				apertures_map[i] = makePoly(point(0, 0),
						aperture->parameter[0] * gscale_to_um,
						aperture->parameter[1], aperture->parameter[2],
						aperture->parameter[3] * gscale_to_um);
				break;
			case GERBV_APTYPE_MACRO: {
				const gerbv_simplified_amacro_t *simplified_amacro =
						aperture->simplified;
				if (!simplified_amacro) {
					std::cerr << "Macro is not simplified" << std::endl;
					continue;
				}
				polygon_set compound;
				while (simplified_amacro) {
					polygon_set part;
					int polarity;
					double rotation;
					switch (simplified_amacro->type) {
					case GERBV_APTYPE_NONE:
					case GERBV_APTYPE_CIRCLE:
					case GERBV_APTYPE_RECTANGLE:
					case GERBV_APTYPE_OVAL:
					case GERBV_APTYPE_POLYGON:
						std::cerr
								<< "Non-macro aperture during macro drawing: skipping"
								<< std::endl;
						simplified_amacro = simplified_amacro->next;
						continue;
					case GERBV_APTYPE_MACRO:
						std::cerr
								<< "Macro start aperture during macro drawing: skipping"
								<< std::endl;
						simplified_amacro = simplified_amacro->next;
						continue;
					case GERBV_APTYPE_MACRO_CIRCLE:
						part = makePoly(
								point(
										simplified_amacro->parameter[2]
												* gscale_to_um,
										simplified_amacro->parameter[3]
												* gscale_to_um),
								simplified_amacro->parameter[1] * gscale_to_um,
								circle_points, 0);
						polarity = simplified_amacro->parameter[0];
						rotation = simplified_amacro->parameter[4];
						break;
					case GERBV_APTYPE_MACRO_OUTLINE: {
						std::list<point> nodes;
						polygon p;
						for (unsigned int i = 0;
								i < round(simplified_amacro->parameter[1]) + 1;
								i++) {
							nodes.push_back(
									point(
											simplified_amacro->parameter[i * 2
													+ 2] * gscale_to_um,
											simplified_amacro->parameter[i * 2
													+ 3] * gscale_to_um));
						}
						boost::polygon::set_points(p, nodes.begin(),
								nodes.end());
						part = p;
						polarity = simplified_amacro->parameter[0];
						rotation = simplified_amacro->parameter[(2
								* int(round(simplified_amacro->parameter[1]))
								+ 4)];
						break;
					}
					case GERBV_APTYPE_MACRO_POLYGON:
						part = makePoly(
								point(
										simplified_amacro->parameter[2]
												* gscale_to_um,
										simplified_amacro->parameter[3]
												* gscale_to_um),
								simplified_amacro->parameter[4] * gscale_to_um,
								simplified_amacro->parameter[1], 0);
						polarity = simplified_amacro->parameter[0];
						rotation = simplified_amacro->parameter[5];
						break;
					case GERBV_APTYPE_MACRO_MOIRE:
						part = makeMoire(
								point(
										simplified_amacro->parameter[0]
												* gscale_to_um,
										simplified_amacro->parameter[1]
												* gscale_to_um),
								simplified_amacro->parameter[2] * gscale_to_um,
								simplified_amacro->parameter[3] * gscale_to_um,
								simplified_amacro->parameter[4] * gscale_to_um,
								simplified_amacro->parameter[5],
								simplified_amacro->parameter[6] * gscale_to_um,
								simplified_amacro->parameter[7] * gscale_to_um);
						polarity = 1;
						rotation = simplified_amacro->parameter[8];
						break;
					case GERBV_APTYPE_MACRO_THERMAL:
						part = makeThermal(
								point(
										simplified_amacro->parameter[0]
												* gscale_to_um,
										simplified_amacro->parameter[1]
												* gscale_to_um),
								simplified_amacro->parameter[2] * gscale_to_um,
								simplified_amacro->parameter[3] * gscale_to_um,
								simplified_amacro->parameter[4] * gscale_to_um);
						polarity = 1;
						rotation = simplified_amacro->parameter[5];
						break;
					case GERBV_APTYPE_MACRO_LINE20:
						part = makeRect(
								point(
										simplified_amacro->parameter[2]
												* gscale_to_um,
										simplified_amacro->parameter[3]
												* gscale_to_um),
								point(
										simplified_amacro->parameter[2]
												* gscale_to_um,
										simplified_amacro->parameter[3]
												* gscale_to_um),
								simplified_amacro->parameter[1] * gscale_to_um);
						polarity = simplified_amacro->parameter[0];
						rotation = simplified_amacro->parameter[6];
						break;
					case GERBV_APTYPE_MACRO_LINE21:
						part = makeRect(
								point(
										simplified_amacro->parameter[3]
												* gscale_to_um,
										simplified_amacro->parameter[4]
												* gscale_to_um),
								simplified_amacro->parameter[1] * gscale_to_um,
								simplified_amacro->parameter[2] * gscale_to_um,
								0, 0);
						polarity = simplified_amacro->parameter[0];
						rotation = simplified_amacro->parameter[5];
						break;
					case GERBV_APTYPE_MACRO_LINE22:
						part =
								makeRect(
										point(
												(simplified_amacro->parameter[3]
														+ simplified_amacro->parameter[1]
																/ 2)
														* gscale_to_um,
												(simplified_amacro->parameter[3]
														+ simplified_amacro->parameter[1]
																/ 2)
														* gscale_to_um),
										simplified_amacro->parameter[1]
												* gscale_to_um,
										simplified_amacro->parameter[2]
												* gscale_to_um, 0, 0);
						polarity = simplified_amacro->parameter[0];
						rotation = simplified_amacro->parameter[5];
						break;
					default:
						std::cerr << "Unrecognized aperture: "
								<< simplified_amacro->type << " skipping"
								<< std::endl;
						simplified_amacro = simplified_amacro->next;
						continue;
					};
					rotate_transform<scalar> rotate(rotation * M_PI / 180);
					boost::polygon::transform(part, rotate);
					if (polarity == 0) {
						compound -= part;
					} else {
						compound += part;
					}
					simplified_amacro = simplified_amacro->next;
				};
				apertures_map[i] = compound;
				break;
			}
			default:
				std::cerr << "Not known aperture " << aperture->type
						<< std::endl;
			}
		}
	}

	bool contour = false;
	std::list<point> contour_points;
	for (gerbv_net_t *currentNet = gerber->netlist; currentNet; currentNet =
			currentNet->next) {
		const point start(currentNet->start_x * gscale_to_um,
				currentNet->start_y * gscale_to_um);
		const point stop(currentNet->stop_x * gscale_to_um,
				currentNet->stop_y * gscale_to_um);

		if (currentNet->interpolation == GERBV_INTERPOLATION_PAREA_START) {
			contour = true;
			continue;
		}
		if (currentNet->interpolation == GERBV_INTERPOLATION_PAREA_END) {
			contour = false;
			polygon p;
			boost::polygon::set_points(p, contour_points.begin(),
					contour_points.end());
			contour_points.clear();
			graph += p;
			continue;
		}

		switch (currentNet->aperture_state) {
		case GERBV_APERTURE_STATE_OFF:
			break; // Nothing to do
		case GERBV_APERTURE_STATE_ON:
			switch (currentNet->interpolation) {
			case GERBV_INTERPOLATION_LINEARx1: {
				if (contour) {
					if (contour_points.empty())
						contour_points.push_back(start);
					contour_points.push_back(stop);
				} else {
					polygon_set trs = apertures_map[currentNet->aperture];
					polygon_set tre = apertures_map[currentNet->aperture];
					boost::polygon::transformation<scalar> ts(start);
					boost::polygon::transformation<scalar> te(stop);
					boost::polygon::transform(trs, ts.invert());
					boost::polygon::transform(tre, te.invert());
					polygon_set gsmall;
					gsmall += trs;
					gsmall += tre;
					switch (gerber->aperture[currentNet->aperture]->type) {
					case GERBV_APTYPE_CIRCLE:
						gsmall +=
								makeRect(start, stop,
										gerber->aperture[currentNet->aperture]->parameter[0]
												* gscale_to_um);
						break;
					default:
						std::cerr << "Aperture "
								<< gerber->aperture[currentNet->aperture]->type
								<< " not implemented in interpolation"
								<< std::endl;
					}
					graph += gsmall;
				}
				break;
			}
			case GERBV_INTERPOLATION_CW_CIRCULAR:
			case GERBV_INTERPOLATION_CCW_CIRCULAR: {
				point center(currentNet->cirseg->cp_x * gscale_to_um,
						currentNet->cirseg->cp_y * gscale_to_um);
				std::list<point> circ;
				//point startd(start);
				//boost::polygon::deconvolve(startd, center);
				//point stopd(start);
				//boost::polygon::deconvolve(stopd, center);
				double startr = boost::polygon::euclidean_distance(start,
						center);
				double stopr = boost::polygon::euclidean_distance(stop, center);
				for (int i = 0; i <= circle_points; i++) {
					double currangle = ((currentNet->cirseg->angle2
							- currentNet->cirseg->angle1)
							* (i / (double) circle_points)
							+ currentNet->cirseg->angle1) * M_PI / 180.0;
					double currr = (stopr - startr)
							* (i / (double) circle_points);
					if (contour) {
						point p(currr * cos(currangle), currr * sin(currangle));
						boost::polygon::convolve(p, center);
						contour_points.push_back(p);
					} else {
						if (gerber->aperture[currentNet->aperture]->type
								== GERBV_APTYPE_CIRCLE) {
							point p(currr * cos(currangle),
									currr * sin(currangle));
							boost::polygon::convolve(p, center);
							double r =
									gerber->aperture[currentNet->aperture]->parameter[0]
											/ 2 * gscale_to_um;
							point c(r * cos(currangle), r * sin(currangle));
							point pn(p);
							point pf(p);
							boost::polygon::convolve(pn, c);
							boost::polygon::deconvolve(pf, c);
							circ.push_front(pn);
							circ.push_back(pf);
						} else {
							std::cerr
									<< "Circular path not Circle not supported"
									<< std::endl;
						}
					}
				}
				if (!contour) {
					polygon_set trs = apertures_map[currentNet->aperture];
					polygon_set tre = apertures_map[currentNet->aperture];
					boost::polygon::transformation<scalar> ts(start);
					boost::polygon::transformation<scalar> te(stop);
					boost::polygon::transform(trs, ts.invert());
					boost::polygon::transform(tre, te.invert());
					polygon_set gsmall;
					gsmall += trs;
					gsmall += tre;
					polygon p;
					boost::polygon::set_points(p, circ.begin(), circ.end());
					gsmall += p;
					graph += gsmall;
				}
				break;
			}
			default:
				std::cerr << "Ignored interpolation "
						<< currentNet->interpolation << std::endl;
			}
			break;
		case GERBV_APERTURE_STATE_FLASH: {
			polygon_set translated = apertures_map[currentNet->aperture];
			boost::polygon::transformation<scalar> transform(stop);
			boost::polygon::transform(translated, transform.invert());
			graph += translated;
			break;
		}
		}
	}

	gerbv_destroy_project(project);
	return graph;
}

polygon_set GerbvFixedDecoder(std::string infile, scalar resolution_in_mm,
		double fixed_dia) {
	polygon_set graph;
	gerbv_project_t *project = gerbv_create_project();
	double gscale_to_um;
	gchar *filename = g_strdup(infile.c_str());
	gerbv_open_layer_from_filename(project, filename);
	g_free(filename);

	if (project->file[0] == NULL) {
		std::cerr << "File not found!" << std::endl;
		gerbv_destroy_project(project);
		return graph;
	}

	gerbv_image_t *gerber = project->file[0]->image;
	if (gerber->info->polarity != GERBV_POLARITY_POSITIVE) {
		std::cerr << "Negative polarity not supported" << std::endl;
		gerbv_destroy_project(project);
		return graph;
	}

	if (gerber->netlist->state->unit == GERBV_UNIT_MM) {
		gscale_to_um = resolution_in_mm;
	} else {
		gscale_to_um = resolution_in_mm * 25.4;
	}

	polygon_set aperture;
	aperture = makePoly(point(0, 0), fixed_dia * resolution_in_mm,
			circle_points, 0);

	for (gerbv_net_t *currentNet = gerber->netlist; currentNet; currentNet =
			currentNet->next) {
		const point start(currentNet->start_x * gscale_to_um,
				currentNet->start_y * gscale_to_um);
		const point stop(currentNet->stop_x * gscale_to_um,
				currentNet->stop_y * gscale_to_um);

		switch (currentNet->aperture_state) {
		case GERBV_APERTURE_STATE_OFF:
			break; // Nothing to do
		case GERBV_APERTURE_STATE_ON:
			std::cerr << "in fixed we support only flash" << std::endl;
			break;
		case GERBV_APERTURE_STATE_FLASH: {
			polygon_set translated = aperture;
			boost::polygon::transformation<scalar> transform(stop);
			boost::polygon::transform(translated, transform.invert());
			graph += translated;
			break;
		}
		}
	}

	gerbv_destroy_project(project);
	return graph;
}
