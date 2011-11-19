#include <SFGUI/RenderQueue.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace sfg {

RenderQueue::RenderQueue() :
	m_display_list( 0 ),
	m_display_list_compiled( false ),
	m_compiled_aabbs( 0 ),
	m_compiled_aabbs_size( 0 ),
	m_checked_view_id( 0 ),
	m_last_cull_passed( false )
{
	m_display_list = glGenLists( 1 );
}

RenderQueue::~RenderQueue() {
	glDeleteLists( m_display_list, 1 );

	if( m_compiled_aabbs ) {
		delete[] m_compiled_aabbs;
	}

	DrawablesVector::iterator d_iter( m_children.begin() );
	DrawablesVector::iterator d_iter_end( m_children.end() );

	for( ; d_iter != d_iter_end; ++d_iter ) {
		delete d_iter->first;
	}
}

void RenderQueue::Add( sf::Shape* shape ) {
	sf::FloatRect aabb( .0f, .0f, .0f, .0f );

	unsigned int points_count = shape->GetPointsCount();

	for( unsigned int point_index = 0; point_index < points_count; ++point_index ) {
		sf::Vector2f point = shape->GetPointPosition( point_index );

		aabb.Left = std::min( point.x, aabb.Left );
		aabb.Top = std::min( point.y, aabb.Top );
		aabb.Width = std::max( point.x - aabb.Left, aabb.Width );
		aabb.Height = std::max( point.y - aabb.Top, aabb.Height );
	}

	m_children.push_back( std::pair<sf::Drawable*, sf::FloatRect>( shape, aabb ) );
}

void RenderQueue::Add( sf::Sprite* sprite ) {
	sf::FloatRect aabb( sprite->GetPosition(), sprite->GetSize() );
	m_children.push_back( std::pair<sf::Drawable*, sf::FloatRect>( sprite, aabb ) );
}

void RenderQueue::Add( sf::Text* text ) {
	sf::FloatRect aabb( text->GetRect() );
	m_children.push_back( std::pair<sf::Drawable*, sf::FloatRect>( text, aabb ) );
}

void RenderQueue::Add( RenderQueue* queue ) {
	DrawablesVector::iterator iter( queue->m_children.begin() );
	DrawablesVector::iterator iter_end( queue->m_children.end() );

	for( ; iter != iter_end; ++iter ) {
		m_children.push_back( *iter );
	}

	queue->m_children.clear();
	delete queue;
}

sf::IntRect* RenderQueue::GetAABBs() const {
	return m_compiled_aabbs;
}

std::size_t RenderQueue::GetAABBSize() const {
	return m_compiled_aabbs_size;
}

void RenderQueue::Compile() {
	if( m_compiled_aabbs ) {
		delete[] m_compiled_aabbs;
	}

	m_compiled_aabbs_size = m_children.size();
	m_compiled_aabbs = new sf::IntRect[ m_compiled_aabbs_size ];

	for( std::size_t index = 0; index < m_compiled_aabbs_size; ++index ) {
		m_compiled_aabbs[ index ] = sf::IntRect( static_cast<int>( m_children[ index ].second.Left + .5f ),
		                                         static_cast<int>( m_children[ index ].second.Top + .5f ),
		                                         static_cast<int>( m_children[ index ].second.Left + m_children[ index ].second.Width + .5f ),
		                                         static_cast<int>( m_children[ index ].second.Top + m_children[ index ].second.Height + .5f ) );
	}
}

void RenderQueue::Render( sf::RenderTarget& target, sf::Renderer& renderer ) const {
	if( !m_display_list ) {
		// Display list couldn't be created, render normally.
		std::size_t children_size = m_children.size();

		for( DrawablesVector::size_type index = 0; index < children_size; ++index ) {
			target.Draw( *( m_children[index].first ) );
		}
	}
	else if( !m_display_list_compiled ) {
		glNewList( m_display_list, GL_COMPILE_AND_EXECUTE );

		std::size_t children_size = m_children.size();

		for( DrawablesVector::size_type index = 0; index < children_size; ++index ) {
			target.Draw( *( m_children[index].first ) );
		}

		glEndList();

		// See what texture SFML bound so we can do the same...
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &m_texture_id );

		m_display_list_compiled = true;
	}
	else {
		// Bind the same texture SFML bound...
		glBindTexture( GL_TEXTURE_2D, m_texture_id );

		glCallList( m_display_list );

		// Unbind if we bound something...
		if( m_texture_id ) {
			renderer.SetTexture( 0 );
		}
	}
}

void RenderQueue::SetPosition( const sf::Vector2f& position ) {
	m_display_list_compiled = false;

	m_checked_view_id = 0;

	sf::Drawable::SetPosition( position );
}

unsigned int RenderQueue::GetCheckedViewID() {
	return m_checked_view_id;
}

void RenderQueue::SetCheckedViewID( unsigned int id ) {
	m_checked_view_id = id;
}

bool RenderQueue::GetCullPass() {
	return m_last_cull_passed;
}

void RenderQueue::SetCullPass( bool result ) {
	m_last_cull_passed = result;
}

}
