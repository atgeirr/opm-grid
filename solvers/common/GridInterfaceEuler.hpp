//===========================================================================
//
// File: GridInterfaceEuler.hpp
//
// Created: Mon Jun 15 12:53:38 2009
//
// Author(s): Atgeirr F Rasmussen <atgeirr@sintef.no>
//            B�rd Skaflestad     <bard.skaflestad@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
Copyright 2009 SINTEF ICT, Applied Mathematics.
Copyright 2009 Statoil ASA.

This file is part of The Open Reservoir Simulator Project (OpenRS).

OpenRS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenRS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenRS.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPENRS_GRIDINTERFACEEULER_HEADER
#define OPENRS_GRIDINTERFACEEULER_HEADER

#include "config.h"
#include <climits>
#include <dune/common/fvector.hh>
#include <dune/grid/common/referenceelements.hh>
#include <boost/iterator/iterator_facade.hpp>

namespace Dune
{


    namespace GIE
    {
	template <class DuneGrid, class EntityPointerType>
	class Cell;

	/// @brief
	/// @todo Doc me!
	/// @tparam
	/// @param
	template <class DuneGrid>
	class Intersection : public boost::iterator_facade<Intersection<DuneGrid>,
							   const Intersection<DuneGrid>,
							   boost::forward_traversal_tag>
	{
	public:
	    /// @brief
	    /// @todo Doc me!
	    typedef typename DuneGrid::LeafIntersectionIterator DuneIntersectionIter;
	    /// @brief
	    /// @todo Doc me!
	    /// @param
	    Intersection(const DuneGrid& grid, DuneIntersectionIter it, int local_index)
		: pgrid_(&grid), iter_(it), local_index_(local_index)
	    {
	    }
	    /// @brief
	    /// @todo Doc me!
	    typedef FieldVector<typename DuneGrid::ctype, DuneGrid::dimension> Vector;
	    typedef FieldVector<typename DuneGrid::ctype, DuneGrid::dimension - 1> LocalVector;
	    typedef typename DuneGrid::ctype Scalar;
	    typedef int Index;
	    typedef GIE::Cell<DuneGrid, typename DuneGrid::template Codim<0>::EntityPointer> Cell;
	    /// @brief
	    /// @todo Doc me!
	    enum { BoundaryMarkerIndex = -1, LocalEndIndex = INT_MAX };

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Scalar area() const
	    {
		return iter_->geometry().volume();
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Vector centroid() const
	    {
		return iter_->geometry().global(localCentroid());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Vector normal() const
	    {
		return iter_->unitOuterNormal(localCentroid());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    bool boundary() const
	    {
		return iter_->boundary();
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    int boundaryId() const
	    {
		return iter_->boundaryId();
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Cell cell() const
	    {
		return Cell(*pgrid_, iter_->inside());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Index cellIndex() const
	    {
		return pgrid_->leafIndexSet().index(*iter_->inside());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Cell neighbourCell() const
	    {
		return Cell(*pgrid_, iter_->outside());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Index neighbourCellIndex() const
	    {
		if (iter_->boundary()) {
		    return BoundaryMarkerIndex;
		} else {
		    return pgrid_->leafIndexSet().index(*iter_->outside());
		}
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Index localIndex() const
	    {
		return local_index_;
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Scalar neighbourCellVolume() const
	    {
		return iter_->outside()->geometry().volume();
	    }

	    /// Used by iterator facade.
	    const Intersection& dereference() const
	    {
		return *this;
	    }
	    /// Used by iterator facade.
	    bool equal(const Intersection& other) const
	    {
		// Note that we do not compare the local_index_ members,
		// since they may or may not be equal for end iterators.
		return iter_ == other.iter_;
	    }
	    /// Used by iterator facade.
	    void increment()
	    {
		++iter_;
		++local_index_;
	    }
	    /// Gives an ordering of intersections.
	    bool operator<(const Intersection& other) const
	    {
		if (cellIndex() == other.cellIndex()) {
		    return localIndex() < other.localIndex();
		} else {
		    return cellIndex() < other.cellIndex();
		}
	    }
	private:
	    const DuneGrid* pgrid_;
	    DuneIntersectionIter iter_;
	    int local_index_;

	    LocalVector localCentroid() const
	    {
		typedef Dune::ReferenceElements<typename DuneGrid::ctype, DuneGrid::dimension - 1> RefElems;
		return RefElems::general(iter_->type()).position(0,0);
	    }

	};


	template <class DuneGrid, class EntityPointerType>
	class Cell
	{
	public:
	    Cell(const DuneGrid& grid, EntityPointerType it)
		: grid_(grid), iter_(it)
	    {
	    }
	    typedef GIE::Intersection<DuneGrid> FaceIterator;
	    typedef typename FaceIterator::Vector Vector;
	    typedef typename FaceIterator::Scalar Scalar;
	    typedef typename FaceIterator::Index Index;

	    FaceIterator facebegin() const
	    {
		return FaceIterator(grid_, iter_->ileafbegin(), 0);
	    }

	    FaceIterator faceend() const
	    {
		return FaceIterator(grid_, iter_->ileafend(), FaceIterator::LocalEndIndex);
	    }

	    Scalar volume() const
	    {
		return iter_->geometry().volume();
	    }

	    Vector centroid() const
	    {
		typedef Dune::ReferenceElements<typename DuneGrid::ctype, DuneGrid::dimension> RefElems;
		Vector localpt
		    = RefElems::general(iter_->type()).position(0,0);
		return iter_->geometry().global(localpt);
	    }

	    Index index() const
	    {
		return grid_.leafIndexSet().index(*iter_);
	    }
	protected:
	    const DuneGrid& grid_;
	    EntityPointerType iter_;
	};


	template <class DuneGrid>
	class CellIterator
	    : public boost::iterator_facade<CellIterator<DuneGrid>,
					    const CellIterator<DuneGrid>,
					    boost::forward_traversal_tag>,
	      public Cell<DuneGrid, typename DuneGrid::template Codim<0>::LeafIterator>
	{
	private:
	    typedef typename DuneGrid::template Codim<0>::LeafIterator DuneCellIter;
	    typedef Cell<DuneGrid, DuneCellIter> CellType;
	public:
	    typedef typename CellType::Vector Vector;
	    typedef typename CellType::Scalar Scalar;
	    typedef typename CellType::Index Index;

	    CellIterator(const DuneGrid& grid, DuneCellIter it)
		: CellType(grid, it)
	    {
	    }
	    /// Used by iterator facade.
	    const CellIterator& dereference() const
	    {
		return *this;
	    }
	    /// Used by iterator facade.
	    bool equal(const CellIterator& other) const
	    {
		return CellType::iter_ == other.CellType::iter_;
	    }
	    /// Used by iterator facade.
	    void increment()
	    {
		++CellType::iter_;
	    }
	};


    } // namespace GIE


    template <class DuneGrid>
    class GridInterfaceEuler
    {
    public:
	typedef GIE::CellIterator<DuneGrid> CellIterator;
	typedef typename CellIterator::Vector Vector;
	typedef typename CellIterator::Scalar Scalar;
	typedef typename CellIterator::Index Index;
	enum { Dimension = DuneGrid::dimension };

	GridInterfaceEuler()
	    : pgrid_(0)
	{
	}
	GridInterfaceEuler(const DuneGrid& grid)
	    : pgrid_(&grid)
	{
	}
	void init(const DuneGrid& grid)
	{
	    pgrid_ = &grid;
	}
	CellIterator cellbegin() const
	{
	    return CellIterator(grid(), grid().template leafbegin<0>());
	}
	CellIterator cellend() const
	{
	    return CellIterator(grid(), grid().template leafend<0>());
	}
        int numberOfCells() const
        {
            return grid().size(0);
        }
	const DuneGrid& grid() const
	{
	    ASSERT(pgrid_);
	    return *pgrid_;
	}
    private:
	const DuneGrid* pgrid_;
    };



} // namespace Dune


#endif // OPENRS_GRIDINTERFACEEULER_HEADER